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

#include "cmsDummyActiveObject.hpp"
#include "cms_cpputest_qf_ctrl.hpp"
#include "qpcpp.hpp"
#include "CppUTest/TestHarness.h"
#include "qp_pkg.hpp"

using namespace cms::test;

TEST_GROUP(qf_ctrlTests)
{
    void setup() final
    {
    }

    void teardown() final
    {
        cms::test::qf_ctrl::Teardown();
    }

    void ConfirmNumberOfPools(uint32_t poolCount)
    {
        CHECK_EQUAL(poolCount, QP::QF::priv_.maxPool_);
    }

    void ConfirmPoolEventSize(uint32_t poolIndex, uint32_t blockSize)
    {
        CHECK_TRUE(poolIndex < QF_MAX_EPOOL);
        CHECK_TRUE(poolIndex < QP::QF::priv_.maxPool_);
        CHECK_EQUAL(blockSize, QP::QF::priv_.ePool_[poolIndex].getBlockSize());
    }
};


TEST(qf_ctrlTests, by_default_setup_creates_three_pubsub_pools)
{
    qf_ctrl::Setup(10, 1000);
    ConfirmNumberOfPools(3);
    ConfirmPoolEventSize(0, 16);
    ConfirmPoolEventSize(1, 80);
    ConfirmPoolEventSize(2, 160);
}

TEST(qf_ctrlTests, setup_will_create_two_pools_when_requested)
{
    qf_ctrl::MemPoolConfigs configs;
    configs.push_back(qf_ctrl::MemPoolConfig {sizeof(uint64_t) * 4, 10});
    configs.push_back(qf_ctrl::MemPoolConfig {sizeof(uint64_t) * 8, 5});

    qf_ctrl::Setup(10, 1000, configs);
    ConfirmNumberOfPools(2);
    ConfirmPoolEventSize(0, sizeof(uint64_t) * 4);
    ConfirmPoolEventSize(1, sizeof(uint64_t) * 8);
}

TEST(qf_ctrlTests, setup_will_create_one_pool_when_requested)
{
    qf_ctrl::MemPoolConfigs configs;
    configs.push_back(qf_ctrl::MemPoolConfig {sizeof(uint64_t) * 10, 10});

    qf_ctrl::Setup(10, 1000, configs);
    ConfirmNumberOfPools(1);
    ConfirmPoolEventSize(0, sizeof(uint64_t) * 10);
}

TEST(qf_ctrlTests, setup_provides_option_to_skip_memory_pool_leak_detection)
{
    qf_ctrl::Setup(10, 1000, qf_ctrl::MemPoolConfigs {},
                   qf_ctrl::MemPoolTeardownOption::IGNORE);

    // purposeful leak of an allocated QEvt
    QP::QEvt* volatile e = Q_NEW(QP::QEvt, 5);
    (void)e;

    // test should pass, as we disabled the memory pool leak detection during
    // teardown.
}

TEST(qf_ctrlTests, provides_modify_memory_pool_leak_detection)
{
    //Do a setup with leak detection.
    qf_ctrl::Setup(10, 1000, qf_ctrl::MemPoolConfigs {},
                   qf_ctrl::MemPoolTeardownOption::CHECK_FOR_LEAKS);

    //but, this test only wants to disable leak detection, likely because of ASSERT
    //testing which may interrupt normal event processing, resulting in false leaks.
    qf_ctrl::ChangeMemPoolTeardownOption(qf_ctrl::MemPoolTeardownOption::IGNORE);

    // purposeful leak of an allocated QEvt
    QP::QEvt* volatile e = Q_NEW(QP::QEvt, 5);
    (void)e;

    // test should pass, as we disabled the memory pool leak detection during
    // teardown.
}

TEST(qf_ctrlTests,
     qf_ctrl_provides_for_ability_to_move_time_forward_firing_active_object_timers_as_expected)
{
    using namespace std::chrono_literals;

    enum Signals { SIG_1 = QP::Q_USER_SIG, SIG_2 };
    qf_ctrl::Setup(10, 1000);

    int sigOneCount = 0;
    int sigTwoCount = 0;

    // a 'dummy' active object is needed to verify
    // that QF timers are actually firing.
    auto dummy = std::unique_ptr<cms::test::DefaultDummyActiveObject>(
      new cms::test::DefaultDummyActiveObject());
    dummy->dummyStart();
    dummy->SetPostedEventHandler([&](QP::QEvt const* e) {
        if (e->sig == SIG_1) {
            sigOneCount++;
        }
        else if (e->sig == SIG_2) {
            sigTwoCount++;
        }
        else {
            TEST_EXIT;
        }
    });

    QP::QTimeEvt singleshotTimer(dummy.get(), SIG_1);
    singleshotTimer.armX(1000, 0);   // single shot timer

    QP::QTimeEvt repeatingTimer(dummy.get(), SIG_2);
    repeatingTimer.armX(2000, 2000);

    // now move time forward 6 seconds. Expect one SIG_1 and
    // 3 hits on SIG_2
    qf_ctrl::MoveTimeForward(6s);

    CHECK_EQUAL(1, sigOneCount);
    CHECK_EQUAL(3, sigTwoCount);
}

TEST(qf_ctrlTests, qf_ctrl_provides_cpputest_for_qpcpp_lib_version)
{
    auto version = qf_ctrl::GetVersion();
    CHECK_TRUE(version != nullptr);

    std::string version_str(version);
    CHECK_FALSE(version_str.empty());
}
