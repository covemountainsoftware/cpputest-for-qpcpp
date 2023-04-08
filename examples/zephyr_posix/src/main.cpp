/*
 * Copyright (c) 2023 Victor Chavez
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <zephyr/kernel.h>
#include <posix_board_if.h>
#include <sm/alarm.hpp>
#include <sm/signals.hpp>
#include <cms_cpputest_qf_ctrl.hpp>
#include <cmsTestPublishedEventRecorder.hpp>
#include <pubsub_signals.hpp>
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>

using namespace cms::test;
static std::array<QP::QEvt const*, 10> testQueueStorage;

/**
 * @brief Demonstration of tests for a QP Active object that is intended
 * 			to be integrated in a zephyr application.
 * 
 */
TEST_GROUP(MeasurementSM)
{
	/*!< Utility to record events from the state machine */
	PublishedEventRecorder* m_recorder = nullptr;
	/*!< device under thest (i.e., the state machine)*/
	sm::Alarm* m_dut;

	/**
	 * @brief Initialize the tests by constructing a new SM
	 * 			and the cms::test API for testing the QP::Active object
	 * 
	 */
    void setup() final
    {
		m_dut = new sm::Alarm();
        qf_ctrl::Setup(MaxSignal+1, 1000);
		m_recorder = PublishedEventRecorder::CreatePublishedEventRecorder(
					qf_ctrl::RECORDER_PRIORITY, QP::Q_USER_SIG, MaxSignal+1);
        m_dut->start(qf_ctrl::UNIT_UNDER_TEST_PRIORITY,
						testQueueStorage.data(),
						testQueueStorage.size(),
                        nullptr, 0U);
		/*  set state machine to its initial state */
		qf_ctrl::ProcessEvents();
    }

	/**
	 * @brief Clear the CPPUTest mocks and delete 
	 * 			any allocated resources for the tests.
	 * 
	 */
    void teardown() final
    {
		qf_ctrl::Teardown();
		mock().clear();
		delete m_recorder;
		delete m_dut;
    }
	
	/**
	 * @brief Wait for the alarm timeout that is initiated
	 * 			when the state machine is started.
	 * 
	 */
	void wait_alarm_timeout()
	{
		k_sleep(K_SECONDS(sm::ALARM_TIME));
		qf_ctrl::ProcessEvents();
		
	}
};

/**
 * @brief Test that the alarm timeout works
 * 
 */
TEST(MeasurementSM, alarm_timeout)
{	
	wait_alarm_timeout();
	CHECK_TRUE(m_recorder->isSignalRecorded(Timeout_SIG));
}

/**
 * @brief Test that resetting the alarm invokes 
 * 			the mockup Alarm driver
 * 
 */
TEST(MeasurementSM, alarm_reset)
{	
	wait_alarm_timeout();
	mock().expectOneCall("AlarmDriver::reset");
	qf_ctrl::PublishAndProcess(Reset_SIG,
                                   m_recorder);
	mock().checkExpectations();		
}


void main(void)
{
	char** av{nullptr};
	auto const test_Res = CommandLineTestRunner::RunAllTests(0, av);
	/* Exit before main ends otherwise zephyr does not exit */
    posix_exit(test_Res);
}