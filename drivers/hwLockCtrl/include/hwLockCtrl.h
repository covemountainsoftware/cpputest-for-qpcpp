/*
MIT License

Copyright (c) <2019-2020> <Matthew Eshleman - https://covemountainsoftware.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/**
 * @brief public header for the Hw Lock Ctrl driver.
 *        This driver controls a physical electronic
 *        controlled hardware lock.
 *
 *        The driver API is fully synchronous.
 */
#ifndef ACTIVEOBJECTUNITTESTINGDEMO_HWLOCKCTRL_H
#define ACTIVEOBJECTUNITTESTINGDEMO_HWLOCKCTRL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief HwLockCtrlSelfTestResult enumerates
 *        possible self test results.
 */
typedef enum HwLockCtrlSelfTestResult
{
    HW_LOCK_CTRL_SELF_TEST_PASSED,
    HW_LOCK_CTRL_SELF_TEST_FAILED_POWER,
    HW_LOCK_CTRL_SELF_TEST_FAILED_MOTOR,
} HwLockCtrlSelfTestResultT;

/**
 * @brief HwLockCtrlInit initializes the driver. Lock state is undefined.
 * @return true - initialization completed successfully.
 *         false - some error.
 */
bool HwLockCtrlInit();

/**
 * @brief  HwLockCtrlLock locks the lock.
 * @return true - lock operation completed successfully
 *         false - some error.
 */
bool HwLockCtrlLock();

/**
 * @brief  HwLockCtrlUnlock unlocks the lock.
 * @return true - unlock operation completed successfully
 *         false - some error.
 */
bool HwLockCtrlUnlock();

/**
 * @brief HwLockCtrlSelfTest executes a self test. When completed, the Lock is always LOCKED.
 * @arg outResult: [out] output the self test results
 * @return true - self test completed and results are available in 'outResult'
 *         false - self test failed to execute.
 */
bool HwLockCtrlSelfTest(HwLockCtrlSelfTestResultT* outResult);

/**
 * @brief HwLockCtrlIsCommOk Confirms communications with the lock without changing lock state.
 * @return true - all good.
 *         false - some unexpected error or communication failed.
 */
bool HwLockCtrlIsCommOk();

#ifdef __cplusplus
}
#endif

#endif //ACTIVEOBJECTUNITTESTINGDEMO_HWLOCKCTRL_H
