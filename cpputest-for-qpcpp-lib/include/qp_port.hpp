//
//
//

#ifndef CPPUTEST_FOR_QPCPP_LIB_QP_PORT_HPP
#define CPPUTEST_FOR_QPCPP_LIB_QP_PORT_HPP

#include <cstdint>    // Exact-width types. C++11 Standard

#ifdef QP_CONFIG
    #include "qp_config.hpp" // external QP configuration
#endif

#ifndef QP_API_VERSION
#define QP_API_VERSION 700 //cpputest for qpcpp support v7.x.x and v8.x.x
#endif

// no-return function specifier (C++11 Standard)
// removed due to certain cpputest test functions
#define Q_NORETURN  void

#define QACTIVE_EQUEUE_TYPE  QEQueue
#define QF_EPOOL_TYPE_  QMPool

// QF critical section for POSIX-QV, see NOTE1
#define QF_CRIT_STAT
#define QF_CRIT_ENTRY()
#define QF_CRIT_EXIT()


// Activate the QF QActive::stop() API
#define QACTIVE_CAN_STOP       1

// QF_LOG2 not defined -- use the internal LOG2() implementation

#include "qequeue.hpp" // QP event queue (for deferring events)
#include "qmpool.hpp"  // QP memory pool (for event pools)
#include "qp.hpp"      // QP platform-independent public interface


namespace QP {

void RunUntilNoReadyActiveObjects();

} // namespace QP

//============================================================================
// interface used only inside QF implementation, but not in applications

#ifdef QP_IMPL

    // QF scheduler locking for POSIX-QV (not needed in single-thread port)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) (static_cast<void>(0))
    #define QF_SCHED_UNLOCK_()    (static_cast<void>(0))

    #define QACTIVE_EQUEUE_WAIT_(me_) \
Q_ASSERT_INCRIT(302, (me_)->m_eQueue.m_frontEvt != nullptr)



#define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
    cpputest_readySet_.insert((me_)->m_prio); \
} while (false)

    // native QF event pool operations
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
(p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
((e_) = static_cast<QEvt *>((p_).get((m_), (qsId_))))
    #define QF_EPOOL_PUT_(p_, e_, qsId_)  ((p_).put((e_), (qsId_)))


namespace QP {
extern QPSet cpputest_readySet_; // ready set of active objects
} // namespace QP

namespace QP {
namespace QF {

} // namespace QF
} // namespace QP

#endif // QP_IMPL

#endif   // CPPUTEST_FOR_QPCPP_LIB_QP_PORT_HPP
