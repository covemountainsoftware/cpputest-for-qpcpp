/// @brief Utility class to simplify creating QEQueue objects.
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

#ifndef CMS_ARRAY_BACKED_QEQUEUE_HPP
#define CMS_ARRAY_BACKED_QEQUEUE_HPP

#include "qpcpp.hpp"
#include <cstdint>
#include <array>

namespace cms {

/// The ArrayBacked QEQueue provides a simplified
/// approach to associating static storage with a QEQueue
/// \tparam MaxEvents
template <std::uint_fast16_t MaxEvents>
class ArrayBackedQEQueue : public QP::QEQueue {
public:
    ArrayBackedQEQueue() : QEQueue(), m_storage()
    {
        m_storage.fill(nullptr);
        init(m_storage.data(), MaxEvents);
    }

    size_t capacity() const
    {
        return m_storage.size() + 1;   // see QEQueue docs for +1 reason
    }

private:
    std::array<QP::QEvt const*, MaxEvents> m_storage;
};

}   // namespace cms

#endif   // CMS_ARRAY_BACKED_QEQUEUE_HPP
