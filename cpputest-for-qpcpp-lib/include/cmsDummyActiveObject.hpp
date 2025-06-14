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
#include <memory>
#include "cmsVectorBackedQEQueue.hpp"
#include "qevtUniquePtr.hpp"
#include "cms_cpputest_qf_ctrl.hpp"

namespace cms {
namespace test {
/// The Dummy Active Object may be used
/// when an active object (AO) under test is
/// interacting with another AO during a test.
template <size_t InternalEventCount>
class DummyActiveObject : public QP::QActive {
public:

    enum class EventBehavior  {
        CALLBACK,  //original behavior, will call the provided callback
        RECORDER   //will record the event
    };

    using PostedEventHandler = std::function<void(QP::QEvt const*)>;

    DummyActiveObject() :
        QP::QActive(Q_STATE_CAST(initial)),
        m_eventHandler(nullptr),
        m_incomingEvents(),
        m_behavior(EventBehavior::CALLBACK),
        m_recordedEvents(0)
    {
        m_incomingEvents.fill(nullptr);
    }

    explicit DummyActiveObject(EventBehavior behavior) :
        QP::QActive(Q_STATE_CAST(initial)),
        m_eventHandler(nullptr),
        m_incomingEvents(),
        m_behavior(behavior),
        m_recordedEvents(100)
    {
        m_incomingEvents.fill(nullptr);

        if (m_behavior == EventBehavior::RECORDER) {
            m_eventHandler = [=](QP::QEvt const* e) {
                this->RecorderEventHandler(e);
            };
        }
    }

    virtual ~DummyActiveObject() = default;

    DummyActiveObject(const DummyActiveObject&)            = delete;
    DummyActiveObject& operator=(const DummyActiveObject&) = delete;
    DummyActiveObject(DummyActiveObject&&)                 = delete;
    DummyActiveObject& operator=(DummyActiveObject&&)      = delete;

    void SetPostedEventHandler(const PostedEventHandler& handler)
    {
        if (m_behavior == EventBehavior::CALLBACK) {
            m_eventHandler = handler;
        }
    }

    void dummyStart(uint_fast8_t priority = 1)
    {
        start(priority, m_incomingEvents.data(), m_incomingEvents.size(),
              nullptr, 0);
    }

    bool isRecorderEmpty() const { return m_recordedEvents.isEmpty(); }

    virtual bool isAnyEventRecorded() const { return !m_recordedEvents.isEmpty(); }

    virtual bool isSignalRecorded(enum_t sig)
    {
        if (!isAnyEventRecorded()) {
            return false;
        }

        const auto e       = getRecordedEvent<QP::QEvt>();
        enum_t recordedSig = e->sig;
        return recordedSig == sig;
    }

    template <class EvtT> cms::QEvtUniquePtr<EvtT> getRecordedEvent()
    {
        if (!isAnyEventRecorded()) {
            return cms::QEvtUniquePtr<EvtT>();
        }

        return cms::QEvtUniquePtr<EvtT>(
          static_cast<QP::QEvt const*>(m_recordedEvents.get(0)));
    }

protected:
    virtual void RecorderEventHandler(QP::QEvt const * e)
    {
        if (e->sig >= QP::Q_USER_SIG) {
            // record the event
            m_recordedEvents.post(e, QP::QF::NO_MARGIN, 0);
        }
    }

    static QP::QState initial(DummyActiveObject* const me,
                              QP::QEvt const* const)
    {
        return me->tran(Q_STATE_CAST(&running));
    }

    static QP::QState running(DummyActiveObject* const me,
                              QP::QEvt const* const e)
    {
        QP::QState rtn;
        switch (e->sig) {
            case Q_INIT_SIG:
                rtn = me->super(&top);
                break;
            case Q_ENTRY_SIG:   // purposeful fall through
            case Q_EXIT_SIG:
                rtn = Q_HANDLED();
                break;
            default:
                if ((me->m_eventHandler != nullptr) && (e->sig != 0)) {
                    me->m_eventHandler(e);
                }
                rtn = me->super(&top);
                break;
        }

        return rtn;
    }

private:
    PostedEventHandler m_eventHandler;
    std::array<QP::QEvt const*, InternalEventCount> m_incomingEvents;
    EventBehavior m_behavior;
    cms::VectorBackedQEQueue m_recordedEvents;
};

using DefaultDummyActiveObject = DummyActiveObject<50>;
using DefaultDummyActiveObjectUniquePtr =
  std::unique_ptr<DefaultDummyActiveObject>;

/**
 * Helper method to create (allocate) and start a dummy active
 * object
 * @param behavior
 * @param priority
 * @return ptr as unique_ptr.
 */
inline DefaultDummyActiveObjectUniquePtr CreateAndStartDummyActiveObject(
  DefaultDummyActiveObject::EventBehavior behavior = DefaultDummyActiveObject::EventBehavior::CALLBACK,
  uint_fast8_t priority = qf_ctrl::DUMMY_AO_A_PRIORITY)
{
    auto dummy = std::make_unique<DefaultDummyActiveObject>(behavior);
    dummy->dummyStart(priority);
    return dummy;
}

}  // namespace test
}   // namespace cms

#endif   // CMS_DUMMY_ACTIVE_OBJECT_HPP
