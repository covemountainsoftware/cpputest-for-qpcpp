/// @brief A sample/demonstration QActive based service used
///        to demonstrate host based unit testing of an active object
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

#include "hwLockCtrlService.hpp"
#include "hwLockCtrl.h"
#include "pubsub_signals.hpp"
#include "hwLockCtrlSelfTestEvent.hpp"
#include "bspTicks.hpp"
#include "qassert.h"

Q_DEFINE_THIS_MODULE("hwLockCtrlService");

using namespace PubSub;

namespace cms {
namespace HwLockCtrl {

    static constexpr uint32_t TICKS_PER_POLL = bsp::TICKS_PER_SECOND * 10; //10 second polling rate

    Service::Service() :
        QActive(initial),
        m_history(nullptr),
        m_poll(this, POLL_COMM_STATUS) {
    }

    Q_STATE_DEF(Service, initial) {
        (void) e;
        HwLockCtrlInit();
        subscribe(HW_LOCK_CTRL_SERVICE_REQUEST_SELF_TEST_SIG);
        subscribe(HW_LOCK_CTRL_SERVICE_REQUEST_LOCKED_SIG);
        subscribe(HW_LOCK_CTRL_SERVICE_REQUEST_UNLOCKED_SIG);
        return tran(&common);
    }

    Q_STATE_DEF(Service, common) {
        QP::QState rtn;
        switch (e->sig) {
            case Q_INIT_SIG:
                rtn = tran(locked);
                break;
            case Q_ENTRY_SIG:
                m_poll.armX(TICKS_PER_POLL, TICKS_PER_POLL);
                rtn = Q_RET_HANDLED;
                break;
            case Q_EXIT_SIG:
                rtn = Q_RET_HANDLED;
                break;
            case HW_LOCK_CTRL_SERVICE_REQUEST_SELF_TEST_SIG:
                rtn = tran(selftest);
                break;
            case HW_LOCK_CTRL_SERVICE_REQUEST_LOCKED_SIG:
                rtn = tran(locked);
                break;
            case HW_LOCK_CTRL_SERVICE_REQUEST_UNLOCKED_SIG:
                rtn = tran(unlocked);
                break;
            case POLL_COMM_STATUS:
                //this is just to demonstrate unit testing of time
                //based functionality. Perhaps a real module would
                // publish a result if this fails, etc.
                HwLockCtrlIsCommOk();
                rtn = Q_RET_HANDLED;
                break;
            case DEMONSTRATE_TEST_OF_QASSERT:
                //demonstrate testing assertions from cpputest
                Q_ASSERT(true == false);
                rtn = Q_RET_HANDLED;
                break;
            default:
                rtn = super(&top);
                break;
        }

        return rtn;
    }

    Q_STATE_DEF(Service, locked) {
        QP::QState rtn;
        switch (e->sig) {
            case Q_ENTRY_SIG:
                HwLockCtrlLock();
                notifyChangedState(LockState::LOCKED);
                rtn = Q_RET_HANDLED;
                break;
            case Q_EXIT_SIG:
                rtn = Q_RET_HANDLED;
                break;
            case HW_LOCK_CTRL_SERVICE_REQUEST_LOCKED_SIG:
                rtn = Q_RET_HANDLED;
                break;
            default:
                rtn = super(&common);
                break;
        }

        return rtn;
    }

    Q_STATE_DEF(Service, unlocked) {
        QP::QState rtn;
        switch (e->sig) {
            case Q_ENTRY_SIG:
                HwLockCtrlUnlock();
                notifyChangedState(LockState::UNLOCKED);
                rtn = Q_RET_HANDLED;
                break;
            case Q_EXIT_SIG:
                rtn = Q_RET_HANDLED;
                break;
            case HW_LOCK_CTRL_SERVICE_REQUEST_UNLOCKED_SIG:
                rtn = Q_RET_HANDLED;
                break;
            default:
                rtn = super(&common);
                break;
        }

        return rtn;
    }

    Q_STATE_DEF(Service, selftest) {
        QP::QState rtn;
        switch (e->sig) {
            case Q_ENTRY_SIG:
                performSelfTest();
                rtn = Q_RET_HANDLED;
                break;
            case REQUEST_GOTO_HISTORY:
                rtn = tran_hist(m_history);
                break;
            case HW_LOCK_CTRL_SERVICE_REQUEST_SELF_TEST_SIG:
                rtn = Q_RET_HANDLED;
                break;
            case POLL_COMM_STATUS:
                //drop during self test
                rtn = Q_RET_HANDLED;
                break;
            default:
                rtn = super(&common);
                break;
        }

        return rtn;
    }

    void Service::performSelfTest() {
        HwLockCtrlSelfTestResult result;
        bool ok = HwLockCtrlSelfTest(&result);
        if (ok && (result == HW_LOCK_CTRL_SELF_TEST_PASSED)) {
            notifySelfTestResult(SelfTestResult::PASS);
        } else {
            notifySelfTestResult(SelfTestResult::FAIL);
        }

        //remind self to transition back to
        //history per this service's requirements
        //note the use of "postLIFO" (urgent) as per:
        //
        // https://covemountainsoftware.com/2020/03/08/uml-statechart-handling-errors-when-entering-a-state/
        //
        static const QP::QEvt event = {REQUEST_GOTO_HISTORY, 0U, 0U};
        postLIFO(&event);
    }

    void Service::notifyChangedState(Service::LockState state) {

        static const QP::QEvt lockedEvent = {HW_LOCK_CTRL_SERVICE_IS_LOCKED_SIG, 0U, 0U};
        static const QP::QEvt unlockedEvent = {HW_LOCK_CTRL_SERVICE_IS_UNLOCKED_SIG, 0U, 0U};

        switch (state) {
            case LockState::LOCKED:
                QP::QF::PUBLISH(&lockedEvent, this);
                m_history = &locked;
                break;
            case LockState::UNLOCKED:
                QP::QF::PUBLISH(&unlockedEvent, this);
                m_history = &unlocked;
                break;
            default:
                Q_ASSERT(true == false);
                break;
        }
    }

    void Service::notifySelfTestResult(SelfTestResult result) {
        switch (result) {
            case SelfTestResult::PASS: //fall through on purpose
            case SelfTestResult::FAIL:
                SelfTestEvent::publish<HW_LOCK_CTRL_SERVICE_SELF_TEST_RESULTS_SIG>(result);
                break;
            default:
                Q_ASSERT(true == false);
                break;
        }
    }

} // namespace HwLockCtrl
} // namespace cms