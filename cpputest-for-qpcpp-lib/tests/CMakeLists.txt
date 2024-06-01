
# prep for cpputest based build
set(TEST_APP_NAME  cpputest-for-qpcpp-lib-tests)
set(TEST_SOURCES
        qassertTests.cpp
        cms_cpputest_qf_ctrlTests.cpp
        cms_cpputest_qf_ctrlPublishTests.cpp
        publishedEventRecorderTests.cpp
        backedQueueTests.cpp
        orthogonalComponentTests.cpp
        orthogonalContainerTests.cpp
        )

# this include expects TEST_SOURCES and TEST_APP_NAME to be
# defined, and creates the cpputest based test executable target
include(${CMS_CMAKE_DIR}/cpputestCMake.cmake)

target_link_libraries(${TEST_APP_NAME} cpputest-for-qpcpp-lib  ${CPPUTEST_LDFLAGS})