/*
 * Copyright (c) 2023 Victor Chavez
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <zephyr/kernel.h>
#include <qpcpp.hpp>
#include <CppUTestExt/MockSupport.h>
#include <sm/signals.hpp>
#include "alarm_driver.hpp"

/*!< Timeout event that will be published by the Alarm timeout*/
static QP::QEvt timeout_evt = QEVT_INITIALIZER(Timeout_SIG);

/**
 * @brief Zephyr timer callback for the alarm
 * 
 * @param pTimer zephyr instance timer
 */
static void timer_cb(k_timer * pTimer)
{
	ARG_UNUSED(pTimer);
	QP::QActive::PUBLISH(&timeout_evt, nullptr);
}

AlarmDriver::AlarmDriver()
{
	k_timer_init(&m_timer, timer_cb, nullptr);
}

AlarmDriver::~AlarmDriver()
{
	k_timer_stop(&m_timer);
}

void AlarmDriver::set_alarm(uint32_t seconds){
	k_timer_start(&m_timer, K_SECONDS(seconds), K_NO_WAIT);
}

void AlarmDriver::reset(){
	k_timer_stop(&m_timer);
	mock().actualCall("AlarmDriver::reset");
}