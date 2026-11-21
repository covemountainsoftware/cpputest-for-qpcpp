#include "cmsDummyActiveObject.hpp"
#include "cms_cpputest_qf_ctrl.hpp"
#include "qpcpp.hpp"
#include <memory>

//cpputest header include must always be last
#include "CppUTest/TestHarness.h"

using namespace cms;
using namespace cms::test;

TEST_GROUP(dummy_ao_tests)
{
    void setup() final
    {
        qf_ctrl::Setup(QP::Q_USER_SIG, 100);
    }

    void teardown() final
    {
        qf_ctrl::Teardown();
    }
};

TEST(dummy_ao_tests, dummy_ao_provides_callback_by_default)
{
    auto dummy = CreateAndStartDummyActiveObject();
    CHECK_TRUE(dummy->isRecorderEmpty());
    CHECK_FALSE(dummy->isAnyEventRecorded());

    static constexpr enum_t TEST1_SIG = QP::Q_USER_SIG + 1;
    enum_t capturedSig = -1;
    dummy->SetPostedEventHandler([&](const QP::QEvt* e){
        capturedSig = e->sig;
    });
    qf_ctrl::PostAndProcess<TEST1_SIG>(dummy.get());
    CHECK_EQUAL(TEST1_SIG, capturedSig);
    CHECK_FALSE(dummy->isAnyEventRecorded()); //still false, internal recorder not in use here.
    CHECK_TRUE(dummy->isRecorderEmpty());
}

TEST(dummy_ao_tests, dummy_ao_provides_recorder_option)
{
    auto dummy = CreateAndStartDummyActiveObject(
      DefaultDummyActiveObject::EventBehavior::RECORDER);

    static constexpr enum_t TEST1_SIG = QP::Q_USER_SIG + 1;
    static constexpr enum_t TEST2_SIG = TEST1_SIG + 1;

    CHECK_TRUE(dummy->isRecorderEmpty());

    enum_t capturedSig = -1;

    //this should actually do nothing, since this AO is using its internal
    //recorder
    dummy->SetPostedEventHandler([&](const QP::QEvt* e){
        capturedSig = e->sig;
    });
    qf_ctrl::PostAndProcess<TEST1_SIG>(dummy.get());
    qf_ctrl::PostAndProcess<TEST2_SIG>(dummy.get());
    CHECK_EQUAL(-1, capturedSig); //confirm above callback did NOT happen

    CHECK_TRUE(dummy->isAnyEventRecorded());
    auto recordedEvent1 = dummy->getRecordedEvent<QP::QEvt>();
    CHECK_TRUE(recordedEvent1 != nullptr);
    CHECK_EQUAL(TEST1_SIG, recordedEvent1->sig);

    CHECK_TRUE(dummy->isAnyEventRecorded());
    CHECK_TRUE(dummy->isNextRecordedSignal(TEST2_SIG));
}
TEST(dummy_ao_tests,
     dummy_ao_in_recorder_mode_will_gc_recorded_events_upon_destruction)
{
    static constexpr enum_t TEST1_SIG = QP::Q_USER_SIG + 1;
    auto dummy = CreateAndStartDummyActiveObject(
      DefaultDummyActiveObject::EventBehavior::RECORDER);

    // a dynamic (pool) event, recorded but never retrieved by the test
    qf_ctrl::PostAndProcess(Q_NEW(QP::QEvt, TEST1_SIG), dummy.get());
    CHECK_TRUE(dummy->isAnyEventRecorded());

    // destroy before Teardown(); Teardown's pool leak check must pass
    dummy.reset();
}

TEST(dummy_ao_tests,
     dummy_ao_deprecated_is_signal_recorded_still_consumes_and_checks_the_oldest_event)
{
    static constexpr enum_t TEST1_SIG = QP::Q_USER_SIG + 1;
    auto dummy = CreateAndStartDummyActiveObject(
      DefaultDummyActiveObject::EventBehavior::RECORDER);
    qf_ctrl::PostAndProcess<TEST1_SIG>(dummy.get());

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    CHECK_TRUE(dummy->isSignalRecorded(TEST1_SIG));
#pragma GCC diagnostic pop
    CHECK_TRUE(dummy->isRecorderEmpty());
}
