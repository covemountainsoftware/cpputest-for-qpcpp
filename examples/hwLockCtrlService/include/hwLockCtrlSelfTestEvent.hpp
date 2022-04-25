/// @brief The HwLockCtrl Self Test Event class.
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

#ifndef HWLOCKCTRLSELFTESTEVENT_HPP
#define HWLOCKCTRLSELFTESTEVENT_HPP

#include "qpcpp.h"
#include "hwLockCtrlSelfTestResultEnum.hpp"

namespace cms {
namespace HwLockCtrl {

    ///event type holding the results of a
    ///HwLockCtrl Service Self Test.
    ///note the handy 'publish' static method.
    class SelfTestEvent : public QP::QEvt {
    public:
        SelfTestResult m_result;

        explicit SelfTestEvent(SelfTestResult result) :
                QP::QEvt(),
                m_result(result)
        {
        }

        template<enum_t sig>
        static void publish(SelfTestResult result)
        {
            auto e = Q_NEW(SelfTestEvent, sig);
            e->m_result = result;
            QP::QF::PUBLISH(e, nullptr);
        }
    };
}
}
#endif //HWLOCKCTRLSELFTESTEVENT_HPP
