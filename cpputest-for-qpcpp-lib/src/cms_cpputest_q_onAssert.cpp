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

#include "cms_cpputest.hpp"
#include "qp_port.hpp"
#include "qsafe.h"
#include "cmsQAssertMockSupport.hpp"
#include "qassert-meta.h"

static bool m_printAssertMeta = true;

void cms::test::QAssertMetaOutputEnable()
{
    m_printAssertMeta = true;
}

void cms::test::QAssertMetaOutputDisable()
{
    m_printAssertMeta = false;
}

void Q_onError(char const* const module, int_t const id)
{
    if (m_printAssertMeta)
    {
        fprintf(stdout, "\n%s(%s:%d)\n", __FUNCTION__ , module, id);
        QAssertMetaDescription meta;
        bool found = QAssertMetaGetDescription(module, id, &meta);
        if (found)
        {
            fprintf(stdout, "Additional details on (%s:%d):  %s\n", module, id, meta.brief);
            fprintf(stdout, "Tips/More:\n%s\n", meta.tips);
            fprintf(stdout, "URL:  %s\n", meta.url);
        }
    }

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
      .actualCall(cms::test::ONERROR_FUNC_NAME)
      .withParameter("module", module)
      .withParameter("id", id);

      CMS_TEST_EXIT;
}
