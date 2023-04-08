/*
 * Copyright (c) 2023 Victor Chavez
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <cstdint>
#include <zephyr/kernel.h>

/**
 * @brief Mockup driver to demonstrate how to integrate
 * 			a QP State machine with CPPUTest for QPCPP
 * 
 */
class AlarmDriver {
	public:
		AlarmDriver();
		/**
		 * @brief Start the alarm timer
		 * @details Internally it uses zephyr k_timer
		 * @param seconds Seconds before alarm is triggered and publishes a Timeout_SIG 
		 */
		void set_alarm(uint32_t seconds);

		/**
		 * @brief Resets the alarm after set_alarm was called
		 * 
		 */
		void reset();
		
		/**
		 * @brief Destroy the Alarm Driver object
		 * 
		 */
		~AlarmDriver();
	private:
		/*!< Internal timer based on zephyr RTOS*/
		k_timer m_timer;
};