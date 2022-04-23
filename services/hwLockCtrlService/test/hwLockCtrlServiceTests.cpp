/// @brief  Tests for the HwLockCtrl::Service, demonstrating various unit testing
///         capabilities of the 'fake' cms::test::qf_ctrl environment, using cpputest.
/// @ingroup
/// @cond
///***************************************************************************
///
/// Copyright (C) 2022 Matthew Eshleman. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, upon written permission from Matthew Eshleman, this program
/// may be distributed and modified under the terms of a Commercial
/// License. For further details, see the Contact Information below.
///
/// Contact Information:
///   Matthew Eshleman
///   https://covemountainsoftware.com
///   info@covemountainsoftware.com
///***************************************************************************
/// @endcond

#include <array>
#include <chrono>
#include "hwLockCtrl.h"
#include "pubsub_signals.hpp"
#include "hwLockCtrlService.hpp"
#include "cms_cpputest_qf_ctrl.hpp"
#include "cmsTestPublishedEventRecorder.hpp"
#include "hwLockCtrlSelfTestEvent.hpp"
#include "bspTicks.hpp"
#include "qassertMockSupport.hpp"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

static constexpr const char *HW_LOCK_CTRL_MOCK = "HwLockCtrl";

using namespace PubSub;
using namespace cms;
using namespace cms::test;

static std::array<QP::QEvt const *, 10> testQueueStorage;

/**
 * @brief These tests demonstrate the following key points:
 *         1) Does NOT test any thread that may be associated with the active object,
 *            rather, the associated cms:test::qf_ctrl environment is thread
 *            free, and "faked" to required the test to drive processing time.
 *         2) Tests the internal behavior of the active object without
 *            knowledge of the internal state machine. Rather, only
 *            by observing the behavior and associated output/results.
 *         3) Follow software engineering best practices, such
 *            as adhering to the DRY principle.
 *         4) Shows how to test if an event was published to the QP framework,
 *            using a published event recorder available from cms::test::qf_ctrl. *
 *         5) Shows how to test if a published event has a custom type and payload.
 *         6) Shows how to test behavior driven by QActive timers (i.e. how
 *            to test the forward movement of time and expected behavior being
 *            tested.)
 */
TEST_GROUP(HwLockCtrlServiceTests) {
    HwLockCtrl::Service* mUnderTest = nullptr;
    test::PublishedEventRecorder* mRecorder = nullptr;

    void setup() final {
        using namespace cms::test;
        qf_ctrl::Setup(MAX_PUB_SIG, bsp::TICKS_PER_SECOND);
        mRecorder = cms::test::PublishedEventRecorder::CreatePublishedEventRecorder(
            qf_ctrl::RECORDER_PRIORITY,
            QP::Q_USER_SIG,
            MAX_PUB_SIG);
        mUnderTest = new HwLockCtrl::Service();
    }

    void teardown() final {
        delete mUnderTest;
        mock().clear();
        qf_ctrl::Teardown();
        delete mRecorder;
    }

    void startServiceToLocked() {
        //setup mock to ensure the service under test calls the driver
        //API as expected.
        mock(HW_LOCK_CTRL_MOCK).expectOneCall("Init");
        mock(HW_LOCK_CTRL_MOCK).expectOneCall("Lock");

        //start the active object under test.
        mUnderTest->start(qf_ctrl::UNIT_UNDER_TEST_PRIORITY,
                          testQueueStorage.data(), testQueueStorage.size(),
                          nullptr, 0U);

        //give the system some processing time to handle
        //any internal or queued events
        giveProcessingTime();

        //check that the driver/mock interactions were all as expected.
        mock().checkExpectations();

        //check that the event recorder captured the expected published event
        CHECK_TRUE(mRecorder->isSignalRecorded(HW_LOCK_CTRL_SERVICE_IS_LOCKED_SIG));
    }

    void startServiceToUnlocked() {
        startServiceToLocked();
        testUnlock();
    }

    void testUnlock() {
        mock(HW_LOCK_CTRL_MOCK).expectOneCall("Unlock");
        qf_ctrl::PublishAndProcess(HW_LOCK_CTRL_SERVICE_REQUEST_UNLOCKED_SIG, mRecorder);
        mock().checkExpectations();
        CHECK_TRUE(mRecorder->isSignalRecorded(HW_LOCK_CTRL_SERVICE_IS_UNLOCKED_SIG));
    }

    void giveProcessingTime() {
        qf_ctrl::ProcessEvents();
    }
};

TEST(HwLockCtrlServiceTests, given_init_when_created_then_does_not_crash) {
    //setup() is automatically called by cpputest, which creates our unit under test
    //fully representing this trivial starting test.
}

TEST(HwLockCtrlServiceTests, given_startup_when_started_then_service_ensures_the_lock_is_locked) {
    //When originally developed, this test contained all the code
    //in the below helper method. Since this "setup" was needed by
    //other tests, it was extracted into a helper method.
    //This pattern of developing tests, discovering the need for common
    //setup helper methods, takes place throughout coding of these unit tests,
    //and represents adhering to the DRY principle, even in our unit testing code.
    startServiceToLocked();
}

TEST(HwLockCtrlServiceTests, given_locked_when_another_lock_request_then_service_is_silent) {
    startServiceToLocked();
    qf_ctrl::PublishAndProcess(HW_LOCK_CTRL_SERVICE_REQUEST_LOCKED_SIG, mRecorder);
    CHECK_TRUE(mRecorder->isEmpty());
    mock().checkExpectations();
}

TEST(HwLockCtrlServiceTests, given_locked_when_unlock_request_then_service_unlocks_the_driver) {
    startServiceToLocked();
    testUnlock();
}

TEST(HwLockCtrlServiceTests, given_unlocked_when_another_unlock_request_then_service_is_silent) {
    startServiceToUnlocked();
    qf_ctrl::PublishAndProcess(HW_LOCK_CTRL_SERVICE_REQUEST_UNLOCKED_SIG, mRecorder);
    CHECK_TRUE(mRecorder->isEmpty());
    mock().checkExpectations();
}

TEST(HwLockCtrlServiceTests, given_unlocked_a_lock_request_will_return_to_locked) {
    startServiceToUnlocked();
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("Lock");
    qf_ctrl::PublishAndProcess(HW_LOCK_CTRL_SERVICE_REQUEST_LOCKED_SIG, mRecorder);
    CHECK_TRUE(mRecorder->isSignalRecorded(HW_LOCK_CTRL_SERVICE_IS_LOCKED_SIG));
    mock().checkExpectations();
}

TEST(HwLockCtrlServiceTests, given_locked_when_selftest_requested_then_service_performs_selftest_publishes_results_and_returns_to_locked) {
    startServiceToLocked();

    auto passed = HW_LOCK_CTRL_SELF_TEST_PASSED;
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("SelfTest").withOutputParameterReturning("outResult", &passed, sizeof(passed));
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("Lock");
    qf_ctrl::PublishAndProcess(HW_LOCK_CTRL_SERVICE_REQUEST_SELF_TEST_SIG, mRecorder);
    mock().checkExpectations();
    auto event = mRecorder->getRecordedEvent<HwLockCtrl::SelfTestEvent>();
    CHECK_TRUE(event != nullptr);
    CHECK_EQUAL(HW_LOCK_CTRL_SERVICE_SELF_TEST_RESULTS_SIG, event->sig);
    CHECK_TRUE(HwLockCtrl::SelfTestResult::PASS == event->m_result);
    CHECK_TRUE(mRecorder->isSignalRecorded(HW_LOCK_CTRL_SERVICE_IS_LOCKED_SIG));
}

TEST(HwLockCtrlServiceTests, given_unlocked_when_selftest_request_then_service_performs_selftest_emits_results_and_returns_to_unlocked) {
    startServiceToUnlocked();

    auto passed = HW_LOCK_CTRL_SELF_TEST_PASSED;
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("SelfTest")
        .withOutputParameterReturning("outResult", &passed, sizeof(passed));
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("Unlock");
    qf_ctrl::PublishAndProcess(HW_LOCK_CTRL_SERVICE_REQUEST_SELF_TEST_SIG, mRecorder);
    mock().checkExpectations();
    auto event = mRecorder->getRecordedEvent<HwLockCtrl::SelfTestEvent>();
    CHECK_TRUE(event != nullptr);
    CHECK_EQUAL(HW_LOCK_CTRL_SERVICE_SELF_TEST_RESULTS_SIG, event->sig);
    CHECK_TRUE(HwLockCtrl::SelfTestResult::PASS == event->m_result);
    CHECK_TRUE(mRecorder->isSignalRecorded(HW_LOCK_CTRL_SERVICE_IS_UNLOCKED_SIG));
}

TEST(HwLockCtrlServiceTests,given_locked_when_selftest_request_which_fails_then_service_still_returns_to_locked) {
    startServiceToLocked();

    auto passed = HW_LOCK_CTRL_SELF_TEST_FAILED_POWER;
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("SelfTest")
        .withOutputParameterReturning("outResult", &passed, sizeof(passed));
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("Lock");
    qf_ctrl::PublishAndProcess(HW_LOCK_CTRL_SERVICE_REQUEST_SELF_TEST_SIG, mRecorder);
    mock().checkExpectations();
    auto event = mRecorder->getRecordedEvent<HwLockCtrl::SelfTestEvent>();
    CHECK_TRUE(event != nullptr);
    CHECK_EQUAL(HW_LOCK_CTRL_SERVICE_SELF_TEST_RESULTS_SIG, event->sig);
    CHECK_TRUE(HwLockCtrl::SelfTestResult::FAIL == event->m_result);
    CHECK_TRUE(mRecorder->isSignalRecorded(HW_LOCK_CTRL_SERVICE_IS_LOCKED_SIG));
}

TEST(HwLockCtrlServiceTests, given_unlocked_when_selftest_request_which_fails_then_service_still_returns_to_unlocked) {
    startServiceToUnlocked();

    auto passed = HW_LOCK_CTRL_SELF_TEST_FAILED_MOTOR;
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("SelfTest").withOutputParameterReturning("outResult", &passed, sizeof(passed));
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("Unlock");
    qf_ctrl::PublishAndProcess(HW_LOCK_CTRL_SERVICE_REQUEST_SELF_TEST_SIG, mRecorder);
    mock().checkExpectations();
    auto event = mRecorder->getRecordedEvent<HwLockCtrl::SelfTestEvent>();
    CHECK_TRUE(event != nullptr);
    CHECK_EQUAL(HW_LOCK_CTRL_SERVICE_SELF_TEST_RESULTS_SIG, event->sig);
    CHECK_TRUE(HwLockCtrl::SelfTestResult::FAIL == event->m_result);
    CHECK_TRUE(mRecorder->isSignalRecorded(HW_LOCK_CTRL_SERVICE_IS_UNLOCKED_SIG));
}

TEST(HwLockCtrlServiceTests, given_locked_when_10secs_passes_then_service_polls_lock_comm_status) {
    using namespace std::chrono_literals;

    startServiceToLocked();
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("IsCommOk");
    qf_ctrl::MoveTimeForward(10s);
    mock().checkExpectations();
}

TEST(HwLockCtrlServiceTests, given_locked_when_9900ms_passes_then_service_has_not_polled_yet) {
    using namespace std::chrono_literals;

    startServiceToLocked();
    mock(HW_LOCK_CTRL_MOCK).expectNoCall("IsCommOk");
    qf_ctrl::MoveTimeForward(9900ms);
    mock().checkExpectations();

    //while here, lets see if it calls the driver upon hitting the 10s mark.
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("IsCommOk");
    qf_ctrl::MoveTimeForward(100ms);
    mock().checkExpectations();
}

TEST(HwLockCtrlServiceTests, given_locked_when_60s_passes_then_service_has_polled_six_times) {
    using namespace std::chrono_literals;

    //Demo multiple events over time, and confirm that the
    //service is polling in an ongoing manner.

    startServiceToLocked();
    mock(HW_LOCK_CTRL_MOCK).expectNCalls(6, "IsCommOk");
    qf_ctrl::MoveTimeForward(60s);
    mock().checkExpectations();
}

TEST(HwLockCtrlServiceTests, given_test_assert_event_will_assert_and_can_be_tested) {
    static const QP::QEvt assertCausingEvent { DEMONSTRATE_TEST_OF_QASSERT, 0, 0 };

    startServiceToLocked();

    cms::test::MockExpectQAssert();
    mUnderTest->POST(&assertCausingEvent, 0);
    giveProcessingTime();
    mock().checkExpectations();
}
