/// @file cpputest_qf_port.cpp
/// @brief QF/C++ port for cpputest host based testing of QF/QP based projects.
/// @cond
/// Modified from the original QP CPP sources by Matthew Eshleman
///***************************************************************************
/// @endcond
///

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #error "Q_SPY not supported in the cpputest port"
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY


namespace QP {

Q_DEFINE_THIS_MODULE("cpputest_qf_port")

/* Global objects ==========================================================*/
QPSet  cpputest_readySet_;  // ready set of active objects

//****************************************************************************
void QF::init() {
    QF_maxPool_ = static_cast<uint_fast8_t>(0);
    bzero(&QF::timeEvtHead_[0],
      static_cast<uint_fast16_t>(sizeof(QF::timeEvtHead_)));
    bzero(&active_[0], static_cast<uint_fast16_t>(sizeof(active_)));
}

#if PURPOSEFULLY_NOT_IMPLEMENTED_
//****************************************************************************
int_t QF::run() {
    Q_ASSERT(true == false); //never 'run' in the cpputest port/environment.
}
#endif

void QF_runUntilNoReadyActiveObjects() {
    while (cpputest_readySet_.notEmpty()) {
        std::uint_fast8_t p = cpputest_readySet_.findMax();
        QActive *a = QF::active_[p];
        QF_CRIT_X_();

        // the active object 'a' must still be registered in QF
        // (e.g., it must not be stopped)
        Q_ASSERT_ID(320, a != nullptr);

        while (!a->m_eQueue.isEmpty()) {
            const auto e = a->m_eQueue.get(0);
            a->dispatch(e, 0);
            QF::gc(e);
        }

        QF_CRIT_E_();

        if (a->m_eQueue.isEmpty()) { /* empty queue? */
            cpputest_readySet_.rmove(p);
        }
    }
}

//............................................................................
void QF::stop() {
    QF::onCleanup();
}

//****************************************************************************
void QActive::start(std::uint_fast8_t const priority,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    (void)stkSize; // unused parameter in the cpputest port

    Q_REQUIRE_ID(600, (0U < priority)  /* priority...*/
        && (priority <= QF_MAX_ACTIVE) /*.. in range */
        && (stkSto == nullptr));   // stack storage must NOT be provided
    m_eQueue.init(qSto, qLen);

    m_prio = static_cast<std::uint8_t>(priority); // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO

    this->init(par, m_prio); // execute initial transition (virtual call)
    QS_FLUSH(); // flush the QS trace buffer to the host
}

//............................................................................
#ifdef QF_ACTIVE_STOP
void QActive::stop() {
    unsubscribeAll();
    cpputest_readySet_.rmove(m_prio);
    QF::remove_(this);
}
#endif

} // namespace QP
