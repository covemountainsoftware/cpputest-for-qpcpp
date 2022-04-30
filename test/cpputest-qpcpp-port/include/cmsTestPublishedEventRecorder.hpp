/// @brief A QActive based class for subscribing to and recording published
///        events for unit testing.
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

#ifndef CMS_TEST_PUBLISHED_EVENT_RECORDER_HPP
#define CMS_TEST_PUBLISHED_EVENT_RECORDER_HPP

#include "cmsDummyActiveObject.hpp"
#include "cmsVectorBackedQEQueue.hpp"
#include "qevtUniquePtr.hpp"
#include "qpcpp.hpp"
#include <vector>

namespace cms {
namespace test {

/// PublishedEventRecorder uses the DummyActiveObject
/// class (a QActive), and subscribes to a desired range of signals.
/// Uses the DummyActiveObject signal callback handler to record
/// desired signals received for the purpose of host PC based
/// unit testing.
class PublishedEventRecorder final : public DefaultDummyActiveObject {
private:
    const enum_t m_startingValue;
    const enum_t m_endValue;
    cms::VectorBackedQEQueue m_recordedEvents;
    enum_t m_oneShotIgnoreSig;

public:
    static PublishedEventRecorder*
    CreatePublishedEventRecorder(uint_fast8_t priority, enum_t startingValue,
                                 enum_t endValue,
                                 size_t maxRecordedEventCount = 100)
    {
        auto recorder = new PublishedEventRecorder(startingValue, endValue,
                                                   maxRecordedEventCount);
        recorder->recorderStart(priority);
        return recorder;
    }

    /// Upon construction and init/start, this active object will
    /// subscribe and record all events in the signal range:
    ///         [startingValue .. endValue)
    /// \param startingValue - starting sig value the recorder will record
    /// \param endValue - final value in sig range to record. This value is
    ///                   NOT recorded, rather "endValue - 1" is recorded.
    ///                   Think of it like the end() iterator.
    /// \param maxRecordedEventCount (default 100) maximum number of events the
    ///                              recorder may store.
    explicit PublishedEventRecorder(enum_t startingValue, enum_t endValue,
                                    size_t maxRecordedEventCount = 100) :
        DummyActiveObject(),
        m_startingValue(startingValue), m_endValue(endValue),
        m_recordedEvents(maxRecordedEventCount), m_oneShotIgnoreSig(0)
    {
    }

    ~PublishedEventRecorder() override
    {
        while (!m_recordedEvents.isEmpty()) {
            const auto e = m_recordedEvents.get(0);
            QP::QF::gc(e);
        }
    };

    PublishedEventRecorder(const PublishedEventRecorder&)            = delete;
    PublishedEventRecorder& operator=(const PublishedEventRecorder&) = delete;
    PublishedEventRecorder(PublishedEventRecorder&&)                 = delete;
    PublishedEventRecorder& operator=(PublishedEventRecorder&&)      = delete;

    void recorderStart(uint_fast8_t priority)
    {
        SetPostedEventHandler(
          [=](QP::QEvt const* e) {
              this->RecorderEventHandler(e);
          });

        dummyStart(priority);

        for (enum_t sig = m_startingValue; sig < m_endValue; ++sig) {
            subscribe(sig);
        }
    }

    bool isEmpty() const { return m_recordedEvents.isEmpty(); }

    bool isAnyEventRecorded() const { return !m_recordedEvents.isEmpty(); }

    bool isSignalRecorded(enum_t sig)
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

    void oneShotIgnoreEvent(enum_t sigToIgnore)
    {
        m_oneShotIgnoreSig = sigToIgnore;
    }

protected:
    void RecorderEventHandler(QP::QEvt const* e)
    {
        if ((e->sig >= m_startingValue) && (e->sig < m_endValue)) {
            if (e->sig == m_oneShotIgnoreSig) {
                m_oneShotIgnoreSig = 0;
            }
            else {
                // record the event
                m_recordedEvents.post(e, QP::QF_NO_MARGIN, 0);
            }
        }
    }
};

}   // namespace test
}   // namespace cms

#endif   // CMS_TEST_PUBLISHED_EVENT_RECORDER_HPP
