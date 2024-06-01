//// @brief  Tests for the qf_ctrl functions for the QF cpputest port.
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

#include "cmsTestPublishedEventRecorder.hpp"
#include "cms_cpputest_qf_ctrl.hpp"
#include "qpcpp.hpp"
#include "CppUTest/TestHarness.h"

using namespace cms::test;

TEST_GROUP(qf_ctrlPublishTests)
{
    static constexpr enum_t TEST1_PUBLISH_SIG = QP::Q_USER_SIG + 1;
    static constexpr enum_t TEST2_PUBLISH_SIG = TEST1_PUBLISH_SIG + 1;

    PublishedEventRecorder* mRecorder = nullptr;

    void setup() final
    {
        qf_ctrl::Setup(TEST2_PUBLISH_SIG * 2, 100);
        mRecorder =
          cms::test::PublishedEventRecorder::CreatePublishedEventRecorder(
            qf_ctrl::RECORDER_PRIORITY, TEST1_PUBLISH_SIG,
            TEST2_PUBLISH_SIG + 1);
    }

    void teardown() final
    {
        cms::test::qf_ctrl::Teardown();
        delete mRecorder;
    }
};

TEST(qf_ctrlPublishTests,
     qf_ctrl_provides_a_trivial_publish_from_test_helper_func)
{
    qf_ctrl::PublishEvent(TEST1_PUBLISH_SIG);   // first published event
    qf_ctrl::PublishEvent(TEST2_PUBLISH_SIG);   // second published event
    qf_ctrl::ProcessEvents();                   // give cpu time for processing
    CHECK_TRUE(
      mRecorder->isSignalRecorded(TEST1_PUBLISH_SIG));   // expect first event
    CHECK_TRUE(
      mRecorder->isSignalRecorded(TEST2_PUBLISH_SIG));   // expect second event
}

TEST(qf_ctrlPublishTests,
     qf_ctrl_provides_a_trivial_publish_and_process_from_test_helper_func)
{
    qf_ctrl::PublishAndProcess(TEST1_PUBLISH_SIG);
    CHECK_TRUE(mRecorder->isSignalRecorded(TEST1_PUBLISH_SIG));
}

TEST(qf_ctrlPublishTests,
     qf_ctrl_provides_a_trivial_publish_and_process_from_test_helper_func_with_recorder_ignore_option)
{
    qf_ctrl::PublishAndProcess(TEST1_PUBLISH_SIG, mRecorder);
    CHECK_FALSE(mRecorder->isSignalRecorded(TEST1_PUBLISH_SIG));
}

TEST(qf_ctrlPublishTests, qf_ctrl_provides_a_qevt_publish_from_test_helper_func)
{
    auto e = Q_NEW(QP::QEvt, TEST1_PUBLISH_SIG);
    qf_ctrl::PublishEvent(e);
    qf_ctrl::ProcessEvents();
    CHECK_TRUE(mRecorder->isSignalRecorded(TEST1_PUBLISH_SIG));
    QP::QF::gc(e);
}

TEST(qf_ctrlPublishTests,
     qf_ctrl_provides_a_qevt_publish_and_process_from_test_helper_func)
{
    auto e = Q_NEW(QP::QEvt, TEST1_PUBLISH_SIG);
    qf_ctrl::PublishAndProcess(e);
    CHECK_TRUE(mRecorder->isSignalRecorded(TEST1_PUBLISH_SIG));
    QP::QF::gc(e);
}

TEST(qf_ctrlPublishTests,
     qf_ctrl_provides_a_qevt_publish_and_process_from_test_helper_func_with_recorder_ignore_option)
{
    auto e = Q_NEW(QP::QEvt, TEST1_PUBLISH_SIG);
    qf_ctrl::PublishAndProcess(e, mRecorder);
    CHECK_FALSE(mRecorder->isSignalRecorded(TEST1_PUBLISH_SIG));
    QP::QF::gc(e);
}
