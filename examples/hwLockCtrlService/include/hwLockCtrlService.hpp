/// @file hwLockCtrlService.hpp
/// @brief A service (active object) for demonstrating cpputest host
///        based unit testing.
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

#ifndef DEMO_HWLOCKCTRLSERVICE_HPP
#define DEMO_HWLOCKCTRLSERVICE_HPP

#include <atomic>
#include "qpcpp.hpp"
#include "hwLockCtrlSelfTestResultEnum.hpp"
#include "pubsub_signals.hpp"

namespace cms {
namespace HwLockCtrl {

/**
 * @brief the HwLockCtrl::Service demonstration active object provides for
 *        higher level hardware lock control behavior.
 *        For example, this service will automatically return the hardware lock
 * to its last state after completing a self test request.
 */
class Service final : public QP::QActive {
public:
    enum class LockState { UNKNOWN, LOCKED, UNLOCKED };

    enum DirectSignals { PING = PubSub::MAX_PUB_SIG, MAX_DIRECT_SIG };

    Service();
    ~Service() override                = default;

    Service(const Service&)            = delete;
    Service& operator=(const Service&) = delete;
    Service(Service&&)                 = delete;
    Service& operator=(Service&&)      = delete;

protected:
    enum InternalSignals {
        REQUEST_GOTO_HISTORY = DirectSignals::MAX_DIRECT_SIG,
        POLL_COMM_STATUS
    };

    Q_STATE_DECL(initial);
    Q_STATE_DECL(common);
    Q_STATE_DECL(locked);
    Q_STATE_DECL(unlocked);
    Q_STATE_DECL(selftest);

private:
    static void notifySelfTestResult(SelfTestResult result);
    void notifyChangedState(LockState state);
    void performSelfTest();

    QP::QStateHandler m_history;
    QP::QTimeEvt m_poll;
};
}   // namespace HwLockCtrl
}   // namespace cms

#endif   // DEMO_HWLOCKCTRLSERVICE_HPP
