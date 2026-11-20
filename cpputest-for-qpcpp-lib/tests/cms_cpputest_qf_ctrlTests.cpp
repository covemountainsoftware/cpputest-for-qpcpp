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
#include <deque>
#include "CppUTest/TestHarness.h"

#define QP_IMPL   // need internal access from QP 8.1.0
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

static void ConfirmNumberOfPools(uint32_t poolCount)
{
    CHECK_EQUAL(poolCount, QP::QF::priv_.maxPool_);
}

static void ConfirmPoolEventSize(uint32_t poolIndex, uint32_t blockSize)
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
    // Do a setup with leak detection.
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
    auto dummy = std::make_unique<cms::test::DefaultDummyActiveObject>();
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

    QP::QTimeEvt singleShotTimer(dummy.get(), SIG_1);
    singleShotTimer.armX(1000, 0);   // single shot timer

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
    const auto version = qf_ctrl::GetVersion();
    CHECK_TRUE(version != nullptr);

    const std::string version_str(version);
    CHECK_FALSE(version_str.empty());
}

namespace {
struct SmallWillRoundEvt : QP::QEvt {
    [[maybe_unused]] uint32_t v;
};
struct LessSmallEvt : QP::QEvt {
    [[maybe_unused]] uint64_t v;
    [[maybe_unused]] uint8_t x;
};

}   // namespace

TEST(
  qf_ctrlTests,
  custom_config_with_unrounded_size_reports_false_leak_when_qp_rounds_the_pool_size)
{
    enum Sigs : enum_t { A_SIG = QP::Q_USER_SIG, B_SIG, C_SIG, MAX_SIG };

    const qf_ctrl::MemPoolConfigs configs {{sizeof(SmallWillRoundEvt), 10}};
    qf_ctrl::Setup(MAX_SIG, 100, configs);
    // no events allocated. Any leak reported is a false positive
}

TEST(
  qf_ctrlTests,
  custom_configs_with_unrounded_size_reports_false_leak_when_qp_rounds_the_pool_size)
{
    enum Sigs : enum_t { A_SIG = QP::Q_USER_SIG, B_SIG, C_SIG, MAX_SIG };

    const qf_ctrl::MemPoolConfigs configs {
      {sizeof(SmallWillRoundEvt), 10},
      {sizeof(LessSmallEvt), 10},
    };
    qf_ctrl::Setup(MAX_SIG, 100, configs);
    // no events allocated. Any leak reported is a false positive
}
TEST(qf_ctrlTests,
     ready_set_from_a_prior_test_does_not_leak_into_the_next_test)
{
    // Simulate a prior test that leaves an active object destroyed
    // (but not stopped) with an event still queued. A static (non-pool)
    // event is used so the prior "test" passes the pool leak check.
    static const QP::QEvt staticEvt(QP::Q_USER_SIG);

    qf_ctrl::Setup(QP::Q_USER_SIG + 1, 100);
    auto* dummy = new DefaultDummyActiveObject();
    dummy->dummyStart();
    dummy->POST(&staticEvt, nullptr);
    delete dummy;
    qf_ctrl::Teardown();

    // Simulate the next, unrelated test. Processing events must not
    // act on stale ready-set state left by the prior test.
    qf_ctrl::Setup(QP::Q_USER_SIG + 1, 100);
    qf_ctrl::ProcessEvents();
}

TEST(qf_ctrlTests,
     process_events_dispatches_in_priority_order_like_qv_one_event_per_pass)
{
    // A low priority AO, while handling event A (with event B already
    // queued), posts to a high priority AO. Per QV/QK semantics, the
    // high priority AO must run before the low priority AO's event B.
    enum Sigs : enum_t { A_SIG = QP::Q_USER_SIG, B_SIG, HI_SIG, MAX_SIG };
    static const QP::QEvt evtA(A_SIG);
    static const QP::QEvt evtB(B_SIG);
    static const QP::QEvt evtHi(HI_SIG);

    qf_ctrl::Setup(MAX_SIG, 100);

    std::deque<enum_t> dispatched;
    auto lo = CreateAndStartDummyActiveObject(
      DefaultDummyActiveObject::EventBehavior::CALLBACK,
      qf_ctrl::DUMMY_AO_A_PRIORITY);
    auto hi = CreateAndStartDummyActiveObject(
      DefaultDummyActiveObject::EventBehavior::CALLBACK,
      qf_ctrl::DUMMY_AO_B_PRIORITY);

    lo->SetPostedEventHandler([&](const QP::QEvt* e) {
        dispatched.push_back(e->sig);
        if (e->sig == A_SIG) {
            hi->POST(&evtHi, nullptr);
        }
    });
    hi->SetPostedEventHandler(
      [&](const QP::QEvt* e) { dispatched.push_back(e->sig); });

    lo->POST(&evtA, nullptr);
    lo->POST(&evtB, nullptr);
    qf_ctrl::ProcessEvents();

    LONGS_EQUAL(3, dispatched.size());
    LONGS_EQUAL(A_SIG, dispatched.front());
    dispatched.pop_front();
    LONGS_EQUAL(HI_SIG, dispatched.front());
    dispatched.pop_front();
    LONGS_EQUAL(B_SIG, dispatched.front());
}

namespace {

/// Counts simulated clock ticks (tick rate 0) using a periodic
/// one-tick time event posted to a dummy active object.
/// Construct after qf_ctrl::Setup().
class TickCounter {
public:
    static constexpr enum_t TICK_SIG = QP::Q_USER_SIG;

    TickCounter() :
        m_dummy(CreateAndStartDummyActiveObject()),
        m_timer(m_dummy.get(), TICK_SIG)
    {
        m_dummy->SetPostedEventHandler(
          [this](QP::QEvt const*) { ++m_count; });
        m_timer.armX(1, 1);
    }

    ~TickCounter() { m_timer.disarm(); }

    TickCounter(const TickCounter&)            = delete;
    TickCounter& operator=(const TickCounter&) = delete;
    TickCounter(TickCounter&&)                 = delete;
    TickCounter& operator=(TickCounter&&)      = delete;

    [[nodiscard]] uint32_t count() const { return m_count; }

private:
    DefaultDummyActiveObjectUniquePtr m_dummy;
    QP::QTimeEvt m_timer;
    uint32_t m_count = 0;
};

}   // namespace

TEST(qf_ctrlTests, move_time_forward_does_not_drop_ticks_to_rounding)
{
    using namespace std::chrono_literals;
    qf_ctrl::Setup(TickCounter::TICK_SIG + 1, 30);
    TickCounter ticks;

    qf_ctrl::MoveTimeForward(1000ms);
    CHECK_EQUAL(30, ticks.count());
}

TEST(qf_ctrlTests, move_time_forward_carries_sub_tick_remainder_across_calls)
{
    using namespace std::chrono_literals;
    qf_ctrl::Setup(TickCounter::TICK_SIG + 1, 100);
    TickCounter ticks;

    // 20 x 5 ms == 100 ms == 10 ticks at 100 Hz
    for (int i = 0; i < 20; ++i) {
        qf_ctrl::MoveTimeForward(5ms);
    }
    CHECK_EQUAL(10, ticks.count());
}

TEST(qf_ctrlTests, move_time_forward_less_than_one_tick_does_not_force_a_tick)
{
    using namespace std::chrono_literals;
    qf_ctrl::Setup(TickCounter::TICK_SIG + 1, 100);
    TickCounter ticks;

    qf_ctrl::MoveTimeForward(5ms);
    CHECK_EQUAL(0, ticks.count());
    qf_ctrl::MoveTimeForward(5ms);
    CHECK_EQUAL(1, ticks.count());
}

TEST(qf_ctrlTests, move_time_forward_remainder_does_not_carry_into_next_test)
{
    using namespace std::chrono_literals;

    // simulate a prior test leaving a 5 ms remainder at 100 Hz
    qf_ctrl::Setup(TickCounter::TICK_SIG + 1, 100);
    qf_ctrl::MoveTimeForward(5ms);
    qf_ctrl::Teardown();

    qf_ctrl::Setup(TickCounter::TICK_SIG + 1, 100);
    TickCounter ticks;
    qf_ctrl::MoveTimeForward(5ms);
    CHECK_EQUAL(0, ticks.count());
}

TEST(qf_ctrlTests, move_ticks_forward_processes_exactly_the_requested_ticks)
{
    qf_ctrl::Setup(TickCounter::TICK_SIG + 1, 100);
    TickCounter ticks;

    qf_ctrl::MoveTicksForward(0);
    CHECK_EQUAL(0, ticks.count());
    qf_ctrl::MoveTicksForward(1);
    CHECK_EQUAL(1, ticks.count());
    qf_ctrl::MoveTicksForward(7);
    CHECK_EQUAL(8, ticks.count());
}
