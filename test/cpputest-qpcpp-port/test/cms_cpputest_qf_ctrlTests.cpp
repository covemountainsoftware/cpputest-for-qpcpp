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

#include "qpcpp.h"
#include "CppUTest/TestHarness.h"
#include "cms_cpputest_qf_ctrl.hpp"

namespace QP {
    extern QMPool QF_pool_[QF_MAX_EPOOL];
    extern std::uint_fast8_t QF_maxPool_;
}

using namespace cms::test;

TEST_GROUP(qf_ctrlTests) {
    void setup() final {
    }

    void teardown() final {
        cms::test::qf_ctrl::Teardown();
    }

    void ConfirmNumberOfPools(uint32_t poolCount) {
        CHECK_EQUAL(poolCount, QP::QF_maxPool_);
    }

    void ConfirmPoolEventSize(uint32_t poolIndex, uint32_t blockSize) {
        CHECK_TRUE(poolIndex < QF_MAX_EPOOL);
        CHECK_TRUE(poolIndex < QP::QF_maxPool_);
        CHECK_EQUAL(blockSize, QP::QF_pool_[poolIndex].getBlockSize());
    }
};

TEST(qf_ctrlTests, by_default_setup_creates_three_pubsub_pools) {
    qf_ctrl::Setup(10, 1000);
    ConfirmNumberOfPools(3);
    ConfirmPoolEventSize(0, 8);
    ConfirmPoolEventSize(1, 40);
    ConfirmPoolEventSize(2, 120);
}

TEST(qf_ctrlTests, setup_will_create_two_pools_when_requested) {
    qf_ctrl::MemPoolConfigs configs;
    configs.push_back(qf_ctrl::MemPoolConfig{ sizeof(uint64_t), 10});
    configs.push_back(qf_ctrl::MemPoolConfig{ sizeof(uint64_t)*2, 5});

    qf_ctrl::Setup(10, 1000, configs);
    ConfirmNumberOfPools(2);
    ConfirmPoolEventSize(0, sizeof (uint64_t));
    ConfirmPoolEventSize(1, sizeof (uint64_t)*2);
}

TEST(qf_ctrlTests, setup_will_create_one_pool_when_requested) {
    qf_ctrl::MemPoolConfigs configs;
    configs.push_back(qf_ctrl::MemPoolConfig{ sizeof(uint64_t)*10, 10});

    qf_ctrl::Setup(10, 1000, configs);
    ConfirmNumberOfPools(1);
    ConfirmPoolEventSize(0, sizeof (uint64_t)*10);
}

TEST(qf_ctrlTests, setup_provides_option_to_skip_memory_pool_leak_detection) {
    qf_ctrl::Setup(10, 1000, qf_ctrl::MemPoolConfigs{}, qf_ctrl::MemPoolTeardownOption::IGNORE);

    //purposeful leak of an allocated QEvt
    QP::QEvt * volatile e = Q_NEW(QP::QEvt, 5);
    (void) e;

    //test should pass, as we disabled the memory pool leak detection during teardown.
}