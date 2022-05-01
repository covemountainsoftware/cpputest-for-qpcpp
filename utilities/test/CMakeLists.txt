
# prep for cpputest based build
set(TEST_APP_NAME utilityGeneralTests)

# files to include in this test target
set(TEST_SOURCES
        orthogonalComponentTests.cpp
        orthogonalContainerTests.cpp
        backedQueueTests.cpp)

# this include expects TEST_SOURCES and TEST_APP_NAME to be
# defined, and creates the cpputest based test executable target
include(${TEST_SUPPORT_TOP_DIR}/common/cpputestCMake.txt)

target_link_libraries(${TEST_APP_NAME} cpputestqpcpplib ${CPPUTEST_LDFLAGS})