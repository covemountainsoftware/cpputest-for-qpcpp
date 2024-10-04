/// @brief  Tests for the cpputest port of the Q_onAssert function.
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

#include "CppUTest/TestHarness.h"
#include "qp_port.hpp"
#include "cmsQAssertMockSupport.hpp"

Q_DEFINE_THIS_MODULE("QAssertTests");

TEST_GROUP(QAssertTests)
{
    void setup() final
    {
    }

    void teardown() final
    {
        mock().clear();
    }
};

TEST(QAssertTests, Q_Assert_results_in_expected_mock_hit_and_proper_test_exit)
{
    cms::test::MockExpectQAssert();
    Q_ASSERT(true == false);
    mock().checkExpectations();
}

TEST(QAssertTests,
     Q_Assert_with_id_results_in_expected_mock_hit_and_proper_test_exit)
{
    constexpr int TEST_ID = 1234;

    cms::test::MockExpectQAssert(Q_this_module_, TEST_ID);
    Q_ASSERT_ID(TEST_ID, true == false);
    mock().checkExpectations();
}
