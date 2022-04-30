/// @brief Trivial 'Ping' and 'Pong' event types, used to demonstrate
///        direct post and response events to/from an active object.
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
#ifndef PING_PONG_EVENTS_HPP
#define PING_PONG_EVENTS_HPP

#include "qpcpp.hpp"

namespace cms {

/// "Ping" - a trivial ping event. An active object
///          supporting this event is expected
///          to respond with a Pong event, using
///          the provided requester and response sig.
///
class Ping : public QP::QEvt {
public:
    QP::QActive* m_requester;
    enum_t responseSig;

    template <enum_t sig>
    static void sendTo(QP::QActive* destination, enum_t responseSig,
                       QP::QActive* requester)
    {
        auto e         = Q_NEW(Ping, sig);
        e->m_requester = requester;
        e->responseSig = responseSig;
        destination->POST(e, 0);
    }
};

/// "Pong" - a trivial pong event, sent in response of
///          an active object that supports a Ping
///          event.
class Pong : public QP::QEvt {
public:
    QP::QActive* m_source;

    static void sendTo(QP::QActive* destination, enum_t pongSig,
                       QP::QActive* source)
    {
        auto e      = Q_NEW(Pong, pongSig);
        e->m_source = source;
        destination->POST(e, 0);
    }
};

}   // namespace cms

#endif   // PING_PONG_EVENTS_HPP
