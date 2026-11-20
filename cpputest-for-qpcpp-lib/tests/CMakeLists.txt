
# prep for cpputest based build
set(TEST_APP_NAME  cpputest-for-qpcpp-lib-tests)
set(TEST_SOURCES
        qassertTests.cpp
        cms_cpputest_qf_ctrlTests.cpp
        cms_cpputest_qf_ctrlPublishTests.cpp
        cms_cpputest_qf_ctrl_post_tests.cpp
        cms_dummy_active_object_tests.cpp
        publishedEventRecorderTests.cpp
        backedQueueTests.cpp
        orthogonalComponentTests.cpp
        orthogonalContainerTests.cpp
        )

# this include expects TEST_SOURCES and TEST_APP_NAME to be
# defined, and creates the cpputest based test executable target
include(${CMS_CMAKE_DIR}/cpputestCMake.cmake)

target_link_libraries(${TEST_APP_NAME} cpputest-for-qpcpp-lib  ${CPPUTEST_LDFLAGS})

# some tests inspect QP internals (qp_pkg.hpp)
target_include_directories(${TEST_APP_NAME} SYSTEM PRIVATE ${CMS_QPCPP_SRC_DIR})
target_compile_options(${TEST_APP_NAME} PRIVATE ${CMS_WARNING_FLAGS})
