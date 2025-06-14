/// @file cpputest_qf_port.cpp
/// @brief QF/C++ port for cpputest host based testing of QF/QP based projects.
/// @cond
/// Modified from the original QP CPP sources by Matthew Eshleman
///***************************************************************************
/// @endcond
///

#define QP_IMPL          // this is QP implementation
#include "qp_port.hpp"   // QF port
#include "qp_pkg.hpp"    // QF package-scope interface
#include "qsafe.h"       // QP embedded systems-friendly assertions
#ifdef Q_SPY             // QS software tracing enabled?
    #error "Q_SPY not supported in the cpputest port"
#else
    #include "qs_dummy.hpp"   // disable the QS software tracing
#endif                        // Q_SPY

namespace QP {

Q_DEFINE_THIS_MODULE("cpputest_qf_port")

/* Global objects ==========================================================*/
QPSet cpputest_readySet_;   // ready set of active objects

//****************************************************************************
void QF::init()
{
    priv_.maxPool_ = static_cast<uint_fast8_t>(0);
    QP::QF::bzero_(&QP::QTimeEvt::timeEvtHead_[0], sizeof(QP::QTimeEvt::timeEvtHead_));
    QP::QF::bzero_(&QP::QActive::registry_[0], sizeof(QP::QActive::registry_));
}

#if PURPOSEFULLY_NOT_IMPLEMENTED_
//****************************************************************************
int_t QF::run()
{
    Q_ASSERT(true == false);   // never 'run' in the cpputest port/environment.
}
#endif

/**
 * Fake cpputest event loop for QActive. Only loops
 * until queue is empty, then removes it from the
 * ready set.
 * @param act an active object to run an event loop upon.
 */
void QActive::evtLoop_(QActive* act)
{
    while (!act->m_eQueue.isEmpty()) {
        QEvt const* e = act->get_();
        act->dispatch(e, act->m_prio);
        QF::gc(e);
    }

    if (act->m_eQueue.isEmpty()) {
        cpputest_readySet_.remove(act->m_prio);
    }
}

void RunUntilNoReadyActiveObjects()
{
    while (cpputest_readySet_.notEmpty()) {
        std::uint_fast8_t p = cpputest_readySet_.findMax();
        #if QP_VERSION > 800
        QActive* a          = QP::QActive::fromRegistry(p);
        #elif QP_VERSION > 700
        QActive* a          = QP::QActive::registry_[p];
        #else
        #error "unsupported QP version"
        #endif

        // the active object 'a' must still be registered in QF
        // (e.g., it must not be stopped)
        Q_ASSERT_ID(320, a != nullptr);

        QActive::evtLoop_(a);
    }
}

//............................................................................
void QF::stop()
{
    QF::onCleanup();
}

//****************************************************************************
void QActive::start(QPrioSpec const prioSpec, QEvt const** const qSto,
                    std::uint_fast16_t const qLen, void* const stkSto,
                    std::uint_fast16_t const stkSize, void const* const par)
{
    // unused parameters in the cpputest port
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU);   // QF-priority
    m_pthre = static_cast<std::uint8_t>(prioSpec >> 8U);     // preemption-thre.
    register_();   // make QF aware of this AO

    m_eQueue.init(qSto, qLen);

    this->init(par, m_prio);   // execute initial transition (virtual call)
    QS_FLUSH();                // flush the QS trace buffer to the host
}

//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive::stop()
{
    unsubscribeAll();
    cpputest_readySet_.remove(m_prio);
    unregister_();
}
#endif

}   // namespace QP
