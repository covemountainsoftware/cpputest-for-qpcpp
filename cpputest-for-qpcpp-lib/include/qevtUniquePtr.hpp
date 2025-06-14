/// @brief An RAII unique_ptr style class for QP::QEvt objects.
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

#ifndef CMS_QEVT_UNIQUE_PTR_HPP
#define CMS_QEVT_UNIQUE_PTR_HPP

#include "qpcpp.hpp"
#include <utility>

namespace cms {

/// I love a nice RAII pattern, such as std::unique_ptr.
/// A QEvt may have been allocated from an event pool, which must be
/// garbage collected. This is easy to forget, and is therefore more
/// error-prone, especially when deferring events or recording events. This
/// class helps guarantee that the event is appropriately garbage collected.
///
/// https://covemountainsoftware.com/2019/11/26/why-i-prefer-c-raii-all-the-things/
///
/// \tparam EvtT - a class/struct derived from QP::QEvt.
template <class EvtT> class QEvtUniquePtr {
public:
    static_assert(std::is_base_of<QP::QEvt, EvtT>::value,
                  "template param 'EvtT' must be a derived class of QP::QEvt");
    constexpr QEvtUniquePtr() noexcept : m_evt(nullptr) { }

    explicit QEvtUniquePtr(QP::QEvt const* const evt) :
        m_evt(static_cast<EvtT const* const>(evt))
    {
    }

    ~QEvtUniquePtr()
    {
        if (m_evt != nullptr) {
            QP::QF::gc(m_evt);
        }
    }

    QEvtUniquePtr(QEvtUniquePtr&& o) noexcept : m_evt(std::move(o.m_evt))
    {
        o.m_evt = nullptr;
    }

    QEvtUniquePtr(QEvtUniquePtr const& rhs)            = delete;
    QEvtUniquePtr& operator=(QEvtUniquePtr const& rhs) = delete;

    EvtT const* operator->() const { return m_evt; }

    explicit operator bool() const { return m_evt != nullptr; }

    bool operator==(void const * ptr) const { return ptr == m_evt; }

    bool operator!=(void const * ptr) const { return ptr != m_evt; }

    const EvtT* get() noexcept { return m_evt; }

private:
    EvtT const* m_evt;
};

}   // namespace cms

#endif   // CMS_QEVT_UNIQUE_PTR_HPP
