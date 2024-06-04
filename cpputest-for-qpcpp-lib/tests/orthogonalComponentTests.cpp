/// @brief Tests for the OrthogonalComponent class.
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

#include "cmsOrthogonalComponent.hpp"
#include "cms_cpputest_qf_ctrl.hpp"
#include "cmsDummyActiveObject.hpp"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

using namespace cms;
using namespace cms::test;

enum InternalTestSigs {
    TEST1_PUBLISH_SIG = QP::Q_USER_SIG + 1,
    TEST2_PUBLISH_SIG,
    TEST3_POST_SIG,
    TEST4_MISC_SIG
};

// since the orthogonal component is a pure virtual base
// class, we must create a test concrete class.
// Using an anonymous namespace to avoid conflicts.
namespace {

class TestComponent : public OrthogonalComponent {
public:
    static constexpr const char* MOCK_NAME = "TestComponent";

    explicit TestComponent(QP::QActive* container) :
        OrthogonalComponent(container, Q_STATE_CAST(initial))
    {
    }

    TestComponent(TestComponent&& other) noexcept :
        OrthogonalComponent(std::move(other))
    {
    }

protected:
    void subscribe() override
    {
        // subscribe to all published events of interest
        // using the container's active object context.
        m_container->subscribe(TEST1_PUBLISH_SIG);
        mock(MOCK_NAME).actualCall("subscribe");
    }

    bool isSignalDesired(enum_t sig) const override
    {
        switch (sig) {
            case TEST1_PUBLISH_SIG:
            case TEST3_POST_SIG:
                return true;
            default:
                return false;
        }
    }

    static QP::QState initial(TestComponent* const me, QP::QEvt const* const)
    {
        return Q_TRAN(&running);
    }

    static QP::QState running(TestComponent* const me, QP::QEvt const* const e)
    {
        QP::QState rtn;
        switch (e->sig) {
            case Q_ENTRY_SIG:   // purposeful fall through
                mock(MOCK_NAME).actualCall("running-entry");
                rtn = Q_HANDLED();
                break;
            case Q_EXIT_SIG:
                rtn = Q_HANDLED();
                break;
            case TEST1_PUBLISH_SIG:
                mock(MOCK_NAME).actualCall("running-TEST1_PUBLISH_SIG");
                rtn = Q_HANDLED();
                break;
            case TEST2_PUBLISH_SIG:
                mock(MOCK_NAME).actualCall("running-TEST2_PUBLISH_SIG");
                rtn = Q_HANDLED();
                break;
            case TEST3_POST_SIG:
                mock(MOCK_NAME).actualCall("running-TEST3_POST_SIG");
                rtn = Q_HANDLED();
                break;
            case TEST4_MISC_SIG:
                mock(MOCK_NAME).actualCall("running-TEST4_MISC_SIG");
                rtn = Q_HANDLED();
                break;
            default:
                rtn = Q_SUPER(&top);
                break;
        }
        return rtn;
    }
};

}   // namespace

TEST_GROUP(OrthogonalComponentTests)
{
    DefaultDummyActiveObject* aoTestCtx = nullptr;
    TestComponent* underTest            = nullptr;

    void setup() final
    {
        qf_ctrl::Setup(TEST2_PUBLISH_SIG * 2, 100);
        aoTestCtx = new DefaultDummyActiveObject();
        aoTestCtx->dummyStart();
    }

    void teardown() final
    {
        mock().clear();
        cms::test::qf_ctrl::Teardown();
        delete aoTestCtx;
        if (underTest != nullptr) {
            delete underTest;
        }
    }

    void createUnitUnderTest()
    {
        underTest = new TestComponent(aoTestCtx);
    }

    void startUnitUnderTest() const
    {
        // we expect the base class to ensure the component
        // has an opportunity to subscribe to events and then
        // enter its initial state.
        mock().strictOrder();
        mock(TestComponent::MOCK_NAME).expectOneCall("subscribe");
        mock(TestComponent::MOCK_NAME).expectOneCall("running-entry");

        underTest->start();
        mock().checkExpectations();
    }
};

TEST(OrthogonalComponentTests, can_create_a_test_component)
{
    createUnitUnderTest();
}

TEST(
  OrthogonalComponentTests,
  upon_starting_a_component_it_can_subscribe_and_perform_initial_hsm_transition)
{
    createUnitUnderTest();
    startUnitUnderTest();
}

TEST(OrthogonalComponentTests, component_will_process_desired_published_signal)
{
    static const QP::QEvt event = QP::QEvt(TEST1_PUBLISH_SIG);

    createUnitUnderTest();
    startUnitUnderTest();

    mock(TestComponent::MOCK_NAME).expectOneCall("running-TEST1_PUBLISH_SIG");
    underTest->componentDispatch(&event);
    mock().checkExpectations();
}

TEST(OrthogonalComponentTests,
     component_will_not_process_undesired_published_signal)
{
    static const QP::QEvt event = QP::QEvt(TEST2_PUBLISH_SIG);

    createUnitUnderTest();
    startUnitUnderTest();

    mock(TestComponent::MOCK_NAME).expectNoCall("running-TEST2_PUBLISH_SIG");
    underTest->componentDispatch(&event);
    mock().checkExpectations();
}

TEST(OrthogonalComponentTests,
     component_will_process_desired_direct_post_signal)
{
    static const QP::QEvt event = QP::QEvt(TEST3_POST_SIG);

    createUnitUnderTest();
    startUnitUnderTest();

    mock(TestComponent::MOCK_NAME).expectOneCall("running-TEST3_POST_SIG");
    underTest->componentDispatch(&event);
    mock().checkExpectations();
}

TEST(OrthogonalComponentTests, component_will_not_process_undesired_misc_signal)
{
    static const QP::QEvt event = QP::QEvt(TEST4_MISC_SIG);

    createUnitUnderTest();
    startUnitUnderTest();

    mock(TestComponent::MOCK_NAME).expectNoCall("running-TEST4_MISC_SIG");
    underTest->componentDispatch(&event);
    mock().checkExpectations();
}

// components also need timer access, but will test that in the Container
// testing context.