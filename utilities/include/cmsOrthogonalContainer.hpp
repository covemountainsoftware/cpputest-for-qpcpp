/// @brief Formal orthogonal component container. See Samek, Chapter 5,
///        section 5.4 Orthogonal Component. This is the generic containing
///        active object context for N orthogonal components.
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

#ifndef CMS_ORTHOGONAL_CONTAINER_HPP
#define CMS_ORTHOGONAL_CONTAINER_HPP

#include <array>
#include <tuple>
#include "cmsOrthogonalComponent.hpp"

namespace cms {

/// The OrthogonalContainer, which is a QP::QActive and a variadic template
/// class, will take 'N' OrthogonalComponents as a template parameter
/// pack. Each parameter must be derived from OrthogonalComponent
/// and each must implement a move constructor.
///
/// See the associated tests (orthogonalContainerTests.cpp) for examples of
/// how to use/create/start a concrete container.
///
/// \tparam Components - the OrthogonalComponent derived classes to be
///                      instantiated and managed by this OrthogonalContainer
template <typename... Components>
class OrthogonalContainer : public QP::QActive {
public:
    static constexpr std::size_t NumberOfComponents = sizeof...(Components);
    static_assert(NumberOfComponents >= 1, "zero components not supported");

    using Tuple = std::tuple<Components...>;

    explicit OrthogonalContainer() :
        QP::QActive(Q_STATE_CAST(initial)), m_components(Components {this}...)
    {
    }

protected:
    // The 'ForEachInTuple' support inspired by:
    // https://stackoverflow.com/questions/26902633/how-to-iterate-over-a-stdtuple-in-c-11
    template <class F, class... Ts, std::size_t... Is>
    static void ForEachInTuple(std::tuple<Ts...>& tuple, F func,
                               std::index_sequence<Is...>)
    {
        using expander = int[];
        (void)expander {0, ((void)func(std::get<Is>(tuple)), 0)...};
    }
    template <class F, class... Ts>
    static void ForEachInTuple(std::tuple<Ts...>& tuple, F func)
    {
        ForEachInTuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
    }

    static QP::QState initial(OrthogonalContainer* const me,
                              QP::QEvt const* const)
    {
        ForEachInTuple(me->m_components,
                       [](auto& component) { component.start(); });
        return Q_TRAN(&running);
    }

    static QP::QState running(OrthogonalContainer* const me,
                              QP::QEvt const* const e)
    {
        QP::QState rtn;
        switch (e->sig) {
            case Q_ENTRY_SIG:   // purposeful fall through
            case Q_EXIT_SIG:
            case Q_INIT_SIG:
                rtn = Q_HANDLED();
                break;
            default: {
                bool handled = false;
                ForEachInTuple(me->m_components,
                               [e, &handled](auto& component) {
                                   handled |= component.componentDispatch(e);
                               });

                if (handled) {
                    rtn = Q_HANDLED();
                }
                else {
                    rtn = Q_SUPER(&top);
                }
            } break;
        }
        return rtn;
    }

private:
    Tuple m_components;
};

}   // namespace cms

#endif   // CMS_ORTHOGONAL_CONTAINER_HPP
