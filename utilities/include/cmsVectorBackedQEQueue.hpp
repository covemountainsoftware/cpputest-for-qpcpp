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

#ifndef CMS_VECTOR_BACKED_QEQUEUE_HPP
#define CMS_VECTOR_BACKED_QEQUEUE_HPP

#include "qpcpp.h"
#include <cstdint>
#include <vector>

namespace cms {

/// The VectorBackedQEQueue provides a simplified
/// approach to associating dynamic storage (one time
/// vector allocation) with a QEQueue.
class VectorBackedQEQueue : public QP::QEQueue {
public:
    explicit VectorBackedQEQueue(std::uint_fast16_t maxEvents) :
            QP::QEQueue(),
            m_storage(maxEvents) {
        std::fill(m_storage.begin(), m_storage.end(), nullptr);
        init(m_storage.data(), m_storage.size());
    }

    size_t capacity() const {
        return m_storage.size() + 1; //see QEQueue docs for +1 reason
    }

private:
    std::vector<QP::QEvt const *> m_storage;
};

}  //namespace cms

#endif //CMS_VECTOR_BACKED_QEQUEUE_HPP
