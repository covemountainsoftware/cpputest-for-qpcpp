//// @brief  Tests for the published event recorder.
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

#include "qpcpp.h"
#include "cms_cpputest_qf_ctrl.hpp"
#include "cmsTestPublishedEventRecorder.hpp"
#include "CppUTest/TestHarness.h"

using namespace cms::test;

TEST_GROUP(PublishedEventRecorderTests) {
    static constexpr enum_t TEST1_PUBLISH_SIG = QP::Q_USER_SIG + 1;
    static constexpr enum_t TEST2_PUBLISH_SIG = TEST1_PUBLISH_SIG + 1;

    PublishedEventRecorder* mUnderTest = nullptr;

    void setup() final {
        qf_ctrl::Setup(TEST2_PUBLISH_SIG*2, 100);
        mUnderTest = new PublishedEventRecorder(TEST1_PUBLISH_SIG, TEST2_PUBLISH_SIG+1);
        mUnderTest->recorderStart(1);
    }

    void teardown() final {
        cms::test::qf_ctrl::Teardown();
        if (mUnderTest)
            delete mUnderTest;
    }

    void ConfirmOneTrivialEventRecordingBehavior(enum_t sig) const {
        qf_ctrl::PublishAndProcess(sig);
        CHECK_TRUE(mUnderTest->isAnyEventRecorded());
        CHECK_FALSE(mUnderTest->isEmpty());
        CHECK_TRUE(mUnderTest->isSignalRecorded(sig));
    }
};

TEST(PublishedEventRecorderTests, recorder_will_capture_trivial_event_in_range) {
    ConfirmOneTrivialEventRecordingBehavior(TEST1_PUBLISH_SIG);
}

TEST(PublishedEventRecorderTests, recorder_will_not_capture_sig_out_of_configured_range) {
    enum_t outOfRecorderRangeSig = TEST1_PUBLISH_SIG + 2;
    qf_ctrl::PublishEvent(outOfRecorderRangeSig);
    qf_ctrl::ProcessEvents();  //give cpu time for processing
    CHECK_FALSE(mUnderTest->isAnyEventRecorded());
    CHECK_TRUE(mUnderTest->isEmpty());
    CHECK_FALSE(mUnderTest->isSignalRecorded(outOfRecorderRangeSig));
}

TEST(PublishedEventRecorderTests, recorder_can_capture_multiple_events) {
    qf_ctrl::PublishEvent(TEST1_PUBLISH_SIG);
    qf_ctrl::PublishEvent(TEST2_PUBLISH_SIG);
    qf_ctrl::ProcessEvents();  //give cpu time for processing
    CHECK_TRUE(mUnderTest->isAnyEventRecorded());
    CHECK_FALSE(mUnderTest->isEmpty());
    CHECK_TRUE(mUnderTest->isSignalRecorded(TEST1_PUBLISH_SIG));
    CHECK_TRUE(mUnderTest->isSignalRecorded(TEST2_PUBLISH_SIG));
}

TEST(PublishedEventRecorderTests, recorder_can_oneshot_ignore_an_event) {
    mUnderTest->oneShotIgnoreEvent(TEST1_PUBLISH_SIG);
    qf_ctrl::PublishAndProcess(TEST1_PUBLISH_SIG);
    CHECK_FALSE(mUnderTest->isAnyEventRecorded());
    CHECK_TRUE(mUnderTest->isEmpty());

    //now, publish again, should record the event this time.
    ConfirmOneTrivialEventRecordingBehavior(TEST1_PUBLISH_SIG);
}

TEST(PublishedEventRecorderTests, upon_destruction_recorder_will_gc_any_recorded_events_not_retrieved) {
    qf_ctrl::PublishEvent(TEST1_PUBLISH_SIG);
    qf_ctrl::PublishEvent(TEST2_PUBLISH_SIG);
    qf_ctrl::ProcessEvents();  //give cpu time for processing
    CHECK_TRUE(mUnderTest->isAnyEventRecorded());
    delete mUnderTest;  //must delete before qf_ctrl teardown leak detection.
    mUnderTest = nullptr;
}

TEST(PublishedEventRecorderTests, recorder_returns_null_if_no_event_was_recorded) {
    auto event = mUnderTest->getRecordedEvent<QP::QEvt>();
    CHECK_TRUE(event == nullptr);
}
