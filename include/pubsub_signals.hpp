///***************************************************************************
/// @brief The public signals used in the publish/subscribe event system
///***************************************************************************

#ifndef PUB_SUB_SIGNALS_HPP
#define PUB_SUB_SIGNALS_HPP

#include "qpcpp.h"

namespace PubSub {

/// The publish/subscribe signals allocated for this project
enum Signals {
    STARTING_PUB_SUB_SIG = QP::Q_USER_SIG,

    #include "hwLockCtrlServiceExpectedPubSubSigs.hpp"

    DEMONSTRATE_TEST_OF_QASSERT,

    MAX_PUB_SIG  // the last published signal
};

} //namespace PubSub

#endif //PUB_SUB_SIGNALS_HPP
