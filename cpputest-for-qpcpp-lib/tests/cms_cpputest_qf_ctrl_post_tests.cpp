//// @brief  Tests for the qf_ctrl post related functions for the QF cpputest port.
/// @ingroup
/// @cond
///***************************************************************************
///
/// Copyright (C) 2024 Matthew Eshleman. All rights reserved.
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

#include "cmsDummyActiveObject.hpp"
#include "cms_cpputest_qf_ctrl.hpp"
#include "qpcpp.hpp"

//cpputest header include must always be last
#include "CppUTest/TestHarness.h"

using namespace cms::test;

TEST_GROUP(qf_ctrl_post_tests)
{
    cms::DefaultDummyActiveObject* mDummy = nullptr;

    void setup() final
    {
        qf_ctrl::Setup(QP::Q_USER_SIG, 100);
        mDummy = new cms::DefaultDummyActiveObject();
        mDummy->dummyStart();
    }

    void teardown() final
    {
        delete mDummy;
        cms::test::qf_ctrl::Teardown();
    }
};

TEST(qf_ctrl_post_tests,
     provides_a_post_and_process_helper_func_with_trivial_signal_enum)
{
    static constexpr enum_t TEST1_SIG = QP::Q_USER_SIG + 1;
    enum_t capturedSig = -1;
    mDummy->SetPostedEventHandler([&](const QP::QEvt* e){
        capturedSig = e->sig;
    });
    qf_ctrl::PostAndProcess<TEST1_SIG>(mDummy);
    CHECK_EQUAL(TEST1_SIG, capturedSig);
}

TEST(qf_ctrl_post_tests,
     provides_a_post_and_process_helper_func)
{
    static constexpr enum_t TEST2_SIG = QP::Q_USER_SIG + 111;
    static const QP::QEvt testEvent = QP::QEvt(TEST2_SIG);
    enum_t capturedSig = -1;
    mDummy->SetPostedEventHandler([&](const QP::QEvt* e){
        capturedSig = e->sig;
    });
    qf_ctrl::PostAndProcess(&testEvent, mDummy);
    CHECK_EQUAL(TEST2_SIG, capturedSig);
}
