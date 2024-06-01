/// @brief Formal orthogonal component. See Samek, Chapter 5,
///        section 5.4 Orthogonal Component.
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

#ifndef CMS_ORTHOGONAL_COMPONENT_HPP
#define CMS_ORTHOGONAL_COMPONENT_HPP

#include <assert.h>
#include "qpcpp.hpp"

namespace cms {

/// The OrthogonalComponent class provides for an interface
/// and standard method to define QHsm components that assume
/// an Orthogonal Component Container parent/owner. This approach
/// enables relatively quick and easy moves of a Component from
/// one active object context to another, as needed by the system.
class OrthogonalComponent : public QP::QHsm {
public:
    explicit OrthogonalComponent(QP::QActive* container,
                                 QP::QStateHandler const initial,
                                 std::uint_fast8_t const qs_id = 0) :
        QP::QHsm(initial),
        m_container(container), m_qs_id(qs_id)
    {
    }

    OrthogonalComponent(const OrthogonalComponent&)             = delete;
    OrthogonalComponent& operator=(const OrthogonalComponent&)  = delete;
    OrthogonalComponent& operator=(OrthogonalComponent&& other) = delete;

    /// @note Due to the way the OrthogonalContainer creates Components,
    ///       a valid move constructor is required
    ///       of any concrete Component
    OrthogonalComponent(OrthogonalComponent&& other) noexcept :
        QP::QHsm(other), m_container(other.m_container), m_qs_id(other.m_qs_id)
    {
    }

    void start()
    {
        assert(m_container != nullptr);
        subscribe();
        init(nullptr, m_qs_id);
    }

    bool componentDispatch(QP::QEvt const* const e)
    {
        assert(m_container != nullptr);
        bool desired = isSignalDesired(e->sig);
        if (desired) {
            QP::QHsm::dispatch(e, m_qs_id);
        }
        return desired;
    }

    /// A concrete Component must implement
    /// this method, where it will return
    /// true for any signal of interest. This includes
    /// signals expected from the pub/sub environment, signals
    /// expected to be posted directly to the container,
    /// or internal signals used for timers or other
    /// purposes.
    virtual bool isSignalDesired(enum_t sig) const = 0;

protected:
    /// A concrete Component must implement
    /// this method, where it will subscribe
    /// to all published signals of interest
    /// to the component, using the available
    /// 'm_container' active object context.
    virtual void subscribe() = 0;

    QP::QActive* m_container;
    std::uint_fast8_t m_qs_id;
};

}   // namespace cms

#endif   // CMS_ORTHOGONAL_COMPONENT_HPP
