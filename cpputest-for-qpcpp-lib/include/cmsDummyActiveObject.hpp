/// @brief A dummy active object, useful for testing interactions
///        in a unit test. Same basic idea as QActiveDummy in
///        QUTest, adapted for CppUTest.
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

#ifndef CMS_DUMMY_ACTIVE_OBJECT_HPP
#define CMS_DUMMY_ACTIVE_OBJECT_HPP

#include "qpcpp.hpp"
#include <array>
#include <functional>
#include <cstddef>

namespace cms {

/// The Dummy Active Object may be used
/// when an active object (AO) under test is
/// interacting with another AO during a test.
template <size_t InternalEventCount>
class DummyActiveObject : public QP::QActive {
public:
    using PostedEventHandler = std::function<void(QP::QEvt const*)>;

    explicit DummyActiveObject() :
        QP::QActive(Q_STATE_CAST(initial)), m_eventHandler(nullptr),
        m_incomingEvents()
    {
        m_incomingEvents.fill(nullptr);
    }

    virtual ~DummyActiveObject() = default;

    DummyActiveObject(const DummyActiveObject&)            = delete;
    DummyActiveObject& operator=(const DummyActiveObject&) = delete;
    DummyActiveObject(DummyActiveObject&&)                 = delete;
    DummyActiveObject& operator=(DummyActiveObject&&)      = delete;

    void SetPostedEventHandler(const PostedEventHandler& handler)
    {
        m_eventHandler = handler;
    }

    void dummyStart(uint_fast8_t priority = 1)
    {
        start(priority, m_incomingEvents.data(), m_incomingEvents.size(),
              nullptr, 0);
    }

protected:
    static QP::QState initial(DummyActiveObject* const me,
                              QP::QEvt const* const)
    {
        return Q_TRAN(&running);
    }

    static QP::QState running(DummyActiveObject* const me,
                              QP::QEvt const* const e)
    {
        QP::QState rtn;
        switch (e->sig) {
            case Q_INIT_SIG:
                rtn = Q_SUPER(&top);
                break;
            case Q_ENTRY_SIG:   // purposeful fall through
            case Q_EXIT_SIG:
                rtn = Q_HANDLED();
                break;
            default:
                if ((me->m_eventHandler != nullptr) && (e->sig != 0)) {
                    me->m_eventHandler(e);
                }
                rtn = Q_SUPER(&top);
                break;
        }

        return rtn;
    }

private:
    PostedEventHandler m_eventHandler;
    std::array<QP::QEvt const*, InternalEventCount> m_incomingEvents;
};

using DefaultDummyActiveObject = DummyActiveObject<50>;

}   // namespace cms

#endif   // CMS_DUMMY_ACTIVE_OBJECT_HPP
