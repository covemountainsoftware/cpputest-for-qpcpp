/// @brief support methods for setup, teardown, of a fake cpputest
///        compatible port of the qpcpp QF framework.
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

#include "cms_cpputest_qf_ctrl.hpp"
#include "cmsTestPublishedEventRecorder.hpp"
#include "qpcpp.hpp"
#include <algorithm>
#include <cassert>
#include <vector>
#include "CppUTest/TestHarness.h"
#include "qp_pkg.hpp"

namespace cms {
namespace test {
namespace qf_ctrl {

static void CreateDefaultPools();
static void CreatePoolConfigsFromArg(const MemPoolConfigs& pubSubEventMemPoolConfigs);

using SubscriberList                       = std::vector<QP::QSubscrList>;
static SubscriberList* l_subscriberStorage = nullptr;

static uint32_t l_ticksPerSecond = 0;

static MemPoolTeardownOption l_memPoolOption = MemPoolTeardownOption::CHECK_FOR_LEAKS;

struct InternalPoolConfig {
    explicit InternalPoolConfig(const MemPoolConfig& conf) :
        config(conf), storage()
    {
        storage.resize(config.eventSize * config.numberOfEvents);
    }
    MemPoolConfig config;
    std::vector<uint8_t> storage;
};
static std::vector<InternalPoolConfig>* l_pubSubEventMemPoolConfigs = nullptr;

void Setup(enum_t const maxPubSubSignalValue, uint32_t ticksPerSecond,
           const MemPoolConfigs& pubSubEventMemPoolConfigs,
           MemPoolTeardownOption memPoolOpt)
{
    using namespace QP;

    assert(l_subscriberStorage == nullptr);
    assert(l_ticksPerSecond == 0);
    assert(l_pubSubEventMemPoolConfigs == nullptr);

    l_memPoolOption     = memPoolOpt;
    l_ticksPerSecond    = ticksPerSecond;
    l_subscriberStorage = new SubscriberList();
    l_subscriberStorage->resize(maxPubSubSignalValue);
    QSubscrList nullValue = QSubscrList();
    std::fill(l_subscriberStorage->begin(), l_subscriberStorage->end(),
              nullValue);

    if (pubSubEventMemPoolConfigs.empty()) {
        CreateDefaultPools();
    }
    else {
        CreatePoolConfigsFromArg(pubSubEventMemPoolConfigs);
    }

    QF::init();
    QF::psInit(l_subscriberStorage->data(), maxPubSubSignalValue);

    for (auto& config : *l_pubSubEventMemPoolConfigs) {
        QF::poolInit(config.storage.data(), config.storage.size(),
                     config.config.eventSize);
    }
}

void Teardown()
{
    using namespace QP;

    delete l_subscriberStorage;
    l_subscriberStorage = nullptr;

    l_ticksPerSecond = 0;

    QF::stop();

    bool leakDetected = false;

    // No test should complete with allocated events sitting
    // in a memory pool.
    if (l_pubSubEventMemPoolConfigs != nullptr) {
        if (l_memPoolOption == MemPoolTeardownOption::CHECK_FOR_LEAKS) {
            for (size_t i = 0; i < l_pubSubEventMemPoolConfigs->size(); ++i) {

                const size_t poolNumOfEvents =
                  l_pubSubEventMemPoolConfigs->at(i).config.numberOfEvents;

                if (poolNumOfEvents != QP::QF::priv_.ePool_[i].getNFree())
                {
                    leakDetected = true;
                    fprintf(stderr, "Memory leak in pool: %zu\n", i);
                }
            }
        }

        delete l_pubSubEventMemPoolConfigs;
        l_pubSubEventMemPoolConfigs = nullptr;

        CHECK_TRUE_TEXT(!leakDetected, "A leak was detected in an internal QF event pool!");
    }
}

void ChangeMemPoolTeardownOption(MemPoolTeardownOption memPoolOpt)
{
    l_memPoolOption = memPoolOpt;
}

void ProcessEvents()
{
    QP::RunUntilNoReadyActiveObjects();
}

void MoveTimeForward(const std::chrono::milliseconds& duration)
{
    assert(l_ticksPerSecond != 0);
    assert(duration.count() >= 0);

    using LoopCounter_t = uint64_t;

    constexpr LoopCounter_t ONCE = 1;

    double millisecondsPerTick = 1000.0 / l_ticksPerSecond;

    // if called, ensure at least one tick is processed
    LoopCounter_t ticks = std::max(
      ONCE, static_cast<LoopCounter_t>(duration.count() / millisecondsPerTick));

    for (LoopCounter_t i = 0; i < ticks; ++i) {
        QP::QTimeEvt::tick(0, nullptr);
        ProcessEvents();
    }
}

void PublishEvent(enum_t sig)
{
    auto e = Q_NEW(QP::QEvt, sig);
    QP::QF::PUBLISH(e, nullptr);
}

void PublishEvent(QP::QEvt const* const e)
{
    QP::QF::PUBLISH(e, nullptr);
}

void PublishAndProcess(enum_t sig, PublishedEventRecorder* recorder)
{
    if (recorder != nullptr) {
        recorder->oneShotIgnoreEvent(sig);
    }

    PublishEvent(sig);
    ProcessEvents();
}

void PublishAndProcess(QP::QEvt const* const e,
                       PublishedEventRecorder* recorder)
{
    if (recorder != nullptr) {
        recorder->oneShotIgnoreEvent(e->sig);
    }

    PublishEvent(e);
    ProcessEvents();
}

void CreateDefaultPools()
{
    //see QP/C++ 7.3.0 release notes, where memory pool behavior/sizing
    //was changed: https://www.state-machine.com/qpcpp/history.html#qpcpp_7_3_0

    l_pubSubEventMemPoolConfigs = new std::vector<InternalPoolConfig>();
    l_pubSubEventMemPoolConfigs->push_back(
      InternalPoolConfig(MemPoolConfig {sizeof(uint64_t) * 2, 25}));
    l_pubSubEventMemPoolConfigs->push_back(
      InternalPoolConfig(MemPoolConfig {sizeof(uint64_t) * 10, 10}));
    l_pubSubEventMemPoolConfigs->push_back(
      InternalPoolConfig(MemPoolConfig {sizeof(uint64_t) * 20, 5}));
}

void CreatePoolConfigsFromArg(const MemPoolConfigs& pubSubEventMemPoolConfigs)
{
    l_pubSubEventMemPoolConfigs = new std::vector<InternalPoolConfig>();
    for (const auto& config : pubSubEventMemPoolConfigs) {
        l_pubSubEventMemPoolConfigs->push_back(InternalPoolConfig(config));
    }
}

const char * GetVersion()
{
    return CPPUTEST_FOR_QPCPP_LIB_VERSION;
}

}   // namespace qf_ctrl
}   // namespace test
}   // namespace cms
