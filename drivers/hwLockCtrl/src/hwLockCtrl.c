/*
 *   This concrete implementation of the LockCtrl "C" style interface
 *   is for demo purposes only. The focus of this project
 *   is how to test state machines and active objects, hence
 *   this fake hardware driver module.
 */
#include "hwLockCtrl.h"
#include <stdio.h>

bool HwLockCtrlInit()
{
    printf("%s() executed\n", __FUNCTION__);
    return true;
}

bool HwLockCtrlLock()
{
    printf("%s() executed\n", __FUNCTION__);
    return true;
}

bool HwLockCtrlUnlock()
{
    printf("%s() executed\n", __FUNCTION__);
    return true;
}

bool HwLockCtrlSelfTest(HwLockCtrlSelfTestResultT* outResult)
{
    printf("%s() executed\n", __FUNCTION__);
    if (outResult)
    {
        *outResult = HW_LOCK_CTRL_SELF_TEST_PASSED;
        return true;
    }
    else
    {
        printf("%s() executed with nullptr arg\n", __FUNCTION__);
        return false;
    }
}

bool HwLockCtrlIsCommOk()
{
    printf("%s() executed\n", __FUNCTION__);
    return true;
}
