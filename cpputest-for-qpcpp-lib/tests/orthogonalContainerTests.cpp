/// @brief Tests for the Orthogonal Container class.
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
#include "cmsOrthogonalContainer.hpp"
#include "cms_cpputest_qf_ctrl.hpp"
#include <memory>
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

using namespace cms;
using namespace cms::test;

enum InternalTestSigs {
    TEST1_PUBLISH_SIG = QP::Q_USER_SIG + 1,
    TEST2_PUBLISH_SIG,
    TEST1_POST_SIG,
    TEST2_POST_SIG,
    TEST1_TIMER_SIG,
    TEST2_TIMER_SIG,
    TEST_MAX_SIG
};

// since the orthogonal component is a pure virtual base
// class, we must create a test concrete class.
// Using an anonymous namespace to avoid conflicts.
namespace {

constexpr uint32_t TICKS_PER_SECOND = 100;

struct MockNames {
    static const char CompOne[];
    static const char CompTwo[];
};

const char MockNames::CompOne[] = "TestOne";
const char MockNames::CompTwo[] = "TestTwo";

template <enum_t SubscribeSig, enum_t PostSig, enum_t TimerSig,
          uint32_t TimerIntervalTicks, const char* MockName>
class TestComponent : public OrthogonalComponent {

private:
    QP::QTimeEvt m_poll;

public:
    static constexpr uint32_t TIMER_INTERVAL_TICKS = TimerIntervalTicks;

    explicit TestComponent(QP::QActive* container) :
        OrthogonalComponent(container, Q_STATE_CAST(initial)),
        m_poll(container, TimerSig)
    {
    }

    // Due to the way the Container creates the desired
    // Components, a properly functioning move
    // constructor is required from any concrete
    // Component
    TestComponent(TestComponent&& other) noexcept :
        OrthogonalComponent(std::move(other)), m_poll(m_container, TimerSig)
    {
    }

protected:
    void subscribe() override
    {
        // subscribe to all published events of interest
        // using the container's active object context.
        m_container->subscribe(SubscribeSig);
        mock(MockName).actualCall("subscribe");
    }

    bool isSignalDesired(enum_t sig) const override
    {
        switch (sig) {
            case SubscribeSig:
            case PostSig:
            case TimerSig:
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
            case Q_ENTRY_SIG:
                me->m_poll.armX(TIMER_INTERVAL_TICKS, TIMER_INTERVAL_TICKS);
                mock(MockName).actualCall("running-entry");
                rtn = Q_HANDLED();
                break;
            case Q_EXIT_SIG:
                rtn = Q_HANDLED();
                break;
            case TEST1_PUBLISH_SIG:
                mock(MockName).actualCall("running-TEST1_PUBLISH_SIG");
                rtn = Q_HANDLED();
                break;
            case TEST2_PUBLISH_SIG:
                mock(MockName).actualCall("running-TEST2_PUBLISH_SIG");
                rtn = Q_HANDLED();
                break;
            case TEST1_POST_SIG:
                mock(MockName).actualCall("running-TEST1_POST_SIG");
                rtn = Q_HANDLED();
                break;
            case TEST2_POST_SIG:
                mock(MockName).actualCall("running-TEST2_POST_SIG");
                rtn = Q_HANDLED();
                break;
            case TEST1_TIMER_SIG:
                mock(MockName).actualCall("running-TEST1_TIMER_SIG");
                rtn = Q_HANDLED();
                break;
            case TEST2_TIMER_SIG:
                mock(MockName).actualCall("running-TEST2_TIMER_SIG");
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

using ComponentOne =
  TestComponent<TEST1_PUBLISH_SIG, TEST1_POST_SIG, TEST1_TIMER_SIG,
                TICKS_PER_SECOND * 2, MockNames::CompOne>;
using ComponentTwo =
  TestComponent<TEST2_PUBLISH_SIG, TEST2_POST_SIG, TEST2_TIMER_SIG,
                TICKS_PER_SECOND * 3, MockNames::CompTwo>;

TEST_GROUP(OrthogonalContainerTests)
{

    std::array<const QP::QEvt*, 20> eventStorage {};

    void setup() final
    {
        eventStorage.fill(nullptr);
        qf_ctrl::Setup(TEST_MAX_SIG, TICKS_PER_SECOND);
    }

    void teardown() final
    {
        mock().clear();
        cms::test::qf_ctrl::Teardown();
    }

    std::unique_ptr<QP::QActive> CreateAndStartWithTwoTestComponents()
    {
        mock().strictOrder();
        mock(MockNames::CompOne).expectOneCall("subscribe");
        mock(MockNames::CompOne).expectOneCall("running-entry");
        mock(MockNames::CompTwo).expectOneCall("subscribe");
        mock(MockNames::CompTwo).expectOneCall("running-entry");
        auto underTest = std::unique_ptr<QP::QActive>(
          new OrthogonalContainer<ComponentOne, ComponentTwo>());
        underTest->start(1, eventStorage.data(), eventStorage.size(), nullptr,
                         0);
        mock().checkExpectations();
        return underTest;
    }

    static QP::QActive* StartAStaticallyAllocatedContainer()
    {
        static std::array<const QP::QEvt*, 20> staticStorage {};
        static OrthogonalContainer<ComponentOne> myContainer;

        mock().strictOrder();
        mock(MockNames::CompOne).expectOneCall("subscribe");
        mock(MockNames::CompOne).expectOneCall("running-entry");

        myContainer.start(1, staticStorage.data(), staticStorage.size(),
                          nullptr, 0);
        mock().checkExpectations();

        return &myContainer;
    }
};

TEST(OrthogonalContainerTests, can_create_a_container_with_single_component)
{
    auto underTest = new OrthogonalContainer<ComponentOne>();
    CHECK_EQUAL(1, underTest->NumberOfComponents);
    delete underTest;
}

TEST(OrthogonalContainerTests, can_create_a_container_with_two_components)
{
    auto underTest = new OrthogonalContainer<ComponentOne, ComponentOne>();
    CHECK_EQUAL(2, underTest->NumberOfComponents);
    delete underTest;
}

TEST(OrthogonalContainerTests, can_start_a_container_with_single_component)
{
    mock().ignoreOtherCalls();
    auto underTest = new OrthogonalContainer<ComponentOne>();
    underTest->start(1, eventStorage.data(), eventStorage.size(), nullptr, 0);
    delete underTest;
}

TEST(OrthogonalContainerTests, can_start_a_container_with_two_components)
{
    mock().ignoreOtherCalls();
    auto underTest = CreateAndStartWithTwoTestComponents();
}

TEST(OrthogonalContainerTests, can_start_a_container_with_three_components)
{
    mock().ignoreOtherCalls();
    auto underTest =
      new OrthogonalContainer<ComponentOne, ComponentOne, ComponentOne>();
    CHECK_EQUAL(3, underTest->NumberOfComponents);
    underTest->start(1, eventStorage.data(), eventStorage.size(), nullptr, 0);
    delete underTest;
}

TEST(OrthogonalContainerTests,
     start_a_container_with_two_components_will_start_both_components)
{
    auto underTest = CreateAndStartWithTwoTestComponents();
}

TEST(OrthogonalContainerTests,
     a_static_container_can_be_started_and_the_timer_works)
{
    StartAStaticallyAllocatedContainer();

    mock(MockNames::CompOne).expectOneCall("running-TEST1_TIMER_SIG");
    qf_ctrl::MoveTimeForward(std::chrono::seconds(2));
    mock().checkExpectations();
}

TEST(
  OrthogonalContainerTests,
  container_with_two_components_will_route_published_event_to_appropriate_components)
{
    auto underTest = CreateAndStartWithTwoTestComponents();

    // TEST1 is expected by Component One
    mock(MockNames::CompOne).expectOneCall("running-TEST1_PUBLISH_SIG");
    qf_ctrl::PublishAndProcess(TEST1_PUBLISH_SIG);
    mock().checkExpectations();

    // TEST2 is expected by Component Two
    mock(MockNames::CompTwo).expectOneCall("running-TEST2_PUBLISH_SIG");
    qf_ctrl::PublishAndProcess(TEST2_PUBLISH_SIG);
    mock().checkExpectations();
}

TEST(OrthogonalContainerTests,
     container_with_two_components_allows_for_timer_in_each)
{
    auto underTest = CreateAndStartWithTwoTestComponents();

    // component one is set to go off in 2 seconds
    mock(MockNames::CompOne).expectOneCall("running-TEST1_TIMER_SIG");
    qf_ctrl::MoveTimeForward(std::chrono::seconds(2));
    mock().checkExpectations();

    // component two is set to go off in 3 seconds, so 1 second after above
    mock(MockNames::CompTwo).expectOneCall("running-TEST2_TIMER_SIG");
    qf_ctrl::MoveTimeForward(std::chrono::seconds(1));
    mock().checkExpectations();
}

TEST(
  OrthogonalContainerTests,
  container_with_two_components_allows_for_posts_for_components_and_expected_components_process_expected_signals)
{
    static const QP::QEvt Test3Event = QP::QEvt(TEST1_POST_SIG);
    static const QP::QEvt Test4Event = QP::QEvt(TEST2_POST_SIG);

    auto underTest = CreateAndStartWithTwoTestComponents();

    mock(MockNames::CompOne).expectOneCall("running-TEST1_POST_SIG");
    mock(MockNames::CompOne).expectNoCall("running-TEST2_POST_SIG");

    mock(MockNames::CompTwo).expectNoCall("running-TEST1_POST_SIG");
    mock(MockNames::CompTwo).expectOneCall("running-TEST2_POST_SIG");

    underTest->POST(&Test3Event, 0);
    underTest->POST(&Test4Event, 0);
    qf_ctrl::ProcessEvents();

    mock().checkExpectations();
}
