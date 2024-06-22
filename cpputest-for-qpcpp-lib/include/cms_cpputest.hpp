//
// Created by eshlemanm on 5/18/24, providing a minor porting layer
// to handle CppUTest 3.8 versus 4.0 changes.
//

#ifndef CPPUTEST_FOR_QPC_CMS_CPPUTEST_HPP
#define CPPUTEST_FOR_QPC_CMS_CPPUTEST_HPP

#include "CppUTest/TestHarness.h"

#ifdef CMS_CPPUTEST_LEGACY
    #define CMS_TEST_EXIT  do { TEST_EXIT } while(0)
#else
  #define CMS_TEST_EXIT  do { TEST_EXIT; } while(0)
#endif    

#endif   // CPPUTEST_FOR_QPC_CMS_CPPUTEST_HPP
