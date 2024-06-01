/// @brief Provides an implementation of the required QP function
/// Q_onAssert(..).
///        This implementation uses cpputest mock and the TEST_EXIT macro
///        which throws an exception. This will generally not work in most
///        of QP, as most methods are marked noexcept.
///
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
#include "qassert.h"
#include "qassertMockSupport.hpp"

Q_NORETURN Q_onAssert(char const* const file, int_t const loc)
{
    // fprintf(stderr, "%s(%s , %d)\n", __FUNCTION__ , file, loc);

    // The TEST_EXIT macro used below is throwing an exception.
    // However, many of QP/QF methods are marked as 'noexcept'
    // so this will not be useful if trying to test
    // any assert that happens in the context of a 'noexcept'.
    //
    // Per https://en.cppreference.com/w/cpp/language/noexcept_spec:
    //   "Non-throwing functions are permitted to call potentially-throwing
    //    functions. Whenever an exception is thrown and the search for a
    //    handler encounters the outermost block of a non-throwing function,
    //    the function std::terminate ... is called ..."
    //
    mock(cms::test::QASSERT_MOCK_NAME)
      .actualCall(cms::test::ONASSERT_FUNC_NAME)
      .withParameter("file", file)
      .withParameter("loc", loc);

    TEST_EXIT;
}
