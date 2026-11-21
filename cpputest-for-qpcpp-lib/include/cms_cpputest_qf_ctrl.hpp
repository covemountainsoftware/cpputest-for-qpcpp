/// @brief Supporting methods for setup, teardown, and control of a fake
///        cpputest compatible port of the QP/C++ (qpcpp) framework.
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

#ifndef CMS_CPPUTEST_QF_CTRL_HPP
#define CMS_CPPUTEST_QF_CTRL_HPP

#include <chrono>
#include "qpcpp.hpp"
#include <vector>

namespace cms::test {

class PublishedEventRecorder;

namespace qf_ctrl {

enum : uint8_t {
    RECORDER_PRIORITY = 1,
    DUMMY_AO_A_PRIORITY,
    DUMMY_AO_B_PRIORITY,
    DUMMY_AO_C_PRIORITY,
    DUMMY_AO_D_PRIORITY,
    DUMMY_AO_E_PRIORITY,
    UNIT_UNDER_TEST_PRIORITY
};
static_assert(UNIT_UNDER_TEST_PRIORITY < QF_MAX_ACTIVE,
              "too many priorities defined");
enum class MemPoolTeardownOption { CHECK_FOR_LEAKS, IGNORE };

struct MemPoolConfig {
    size_t eventSize;
    size_t numberOfEvents;
};

using MemPoolConfigs = std::vector<MemPoolConfig>;

/// Setup the QP/QF subsystem for a unit test.
/// \param maxPubSubSignalValue - the system under test's maximum pub/sub
///                               signal value.
/// \param ticksPerSecond - the system under test's expected ticks per second.
/// \param pubSubEventMemPoolConfigs :
///           empty list for internal defaults. Otherwise provide the desired
///           configs. Provided configs must be ordered from smallest to largest
///           per QP requirements.
/// \param memPoolOpt - should a Teardown check for leaks in the pub/sub event
///                     memory pools?
void Setup(enum_t maxPubSubSignalValue, uint32_t ticksPerSecond,
           const MemPoolConfigs& pubSubEventMemPoolConfigs = {},
           MemPoolTeardownOption memPoolOpt = MemPoolTeardownOption::CHECK_FOR_LEAKS);

void ChangeMemPoolTeardownOption(MemPoolTeardownOption memPoolOpt);

/// Teardown the QP/QF subsystem after completing a unit test.
/// Destroy any active objects or recorders holding pool events
/// BEFORE calling Teardown(), so their events are returned to the
/// pools prior to leak detection.
void Teardown();

/// \return true between Setup() and Teardown(), i.e. while the QF
///         event pools are valid.
bool IsSetup();

/// During a unit test, call this function to "give CPU time"
/// to the QF subsystem.
void ProcessEvents();

/// During a unit test, call this function to "move time forward" by
/// an exact number of system clock ticks (tick rate 0). Each tick
/// executes the QF framework's tick function followed by ProcessEvents().
/// \param ticks - how many ticks should be simulated. Zero is a no-op.
/// \note Only QP tick rate 0 is simulated. Time events armed on any other
///       tick rate (QF_MAX_TICK_RATE > 1) never fire in this port.
void MoveTicksForward(uint32_t ticks);

/// During a unit test, call this function to "move time forward."
/// Converts the duration to ticks using the ticksPerSecond provided to
/// Setup() and calls MoveTicksForward(). Time less than one tick is not
/// lost: the remainder carries into the next call (until Teardown()).
/// For example, at 100 Hz, two calls of 5 ms each produce one tick.
/// No tick is forced; to guarantee a tick, use MoveTicksForward(1).
/// \param duration - how many milliseconds of time should be simulated
/// \note Only QP tick rate 0 is simulated (see MoveTicksForward()).
void MoveTimeForward(const std::chrono::milliseconds &duration);

/// Publish a trivial QEvt event to the QF framework. This helper
/// method allocates the event, sets the signal value, and
/// publishes to the QF framework.
/// \param sig
/// \note must call ProcessEvents() at some point afterwards for any
///       active objects under test to receive the event.
void PublishEvent(enum_t sig);
void PublishEvent(QP::QEvt const * e);

/// Publish a trivial QEvt event to the QF framework followed internally
/// by ProcessEvents().
/// \param sig
/// \param recorder when provided, will automatically ensure the published
///                  event is ignored by the recorder.
void PublishAndProcess(enum_t sig,
                       PublishedEventRecorder* recorder = nullptr);
void PublishAndProcess(QP::QEvt const * e,
                       PublishedEventRecorder* recorder = nullptr);

/// Helper method to Post an event to an active object
/// followed internally by ProcessEvents().
/// \param e
/// \param dest
inline void PostAndProcess(QP::QEvt const * e, QP::QActive* dest)
{
    dest->POST(e, nullptr);
    ProcessEvents();
}

/// Helper method to Post a static const QEvt to an active object
/// followed internally by ProcessEvents().
/// \tparam sig (template param):  the signal value of the const event to post
/// \param dest : the active object to post to.
template <enum_t sig>
inline void PostAndProcess(QP::QActive* dest)
{
    static const QP::QEvt constEvent = QP::QEvt(sig);
    PostAndProcess(&constEvent, dest);
}

/// Get the internal library version string.
/// Uses semantic versioning.
const char * GetVersion();

} //namespace qf_ctrl
} // namespace cms::test

#endif //CMS_CPPUTEST_QF_CTRL_HPP
