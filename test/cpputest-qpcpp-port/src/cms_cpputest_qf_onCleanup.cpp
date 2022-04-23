
#include "qpcpp.h"
#include "CppUTest/TestHarness.h"

namespace QP {

extern QMPool QF_pool_[QF_MAX_EPOOL];
extern std::uint_fast8_t QF_maxPool_;

void QF::onCleanup() {
    for (int i = 0; i < QF_maxPool_; ++i) {
        CHECK_TRUE_TEXT(QF_pool_[i].m_nTot == QF_pool_[i].m_nFree,
                        "A leak was detected in an internal QF event pool!");
    }
}

} //namespace QP