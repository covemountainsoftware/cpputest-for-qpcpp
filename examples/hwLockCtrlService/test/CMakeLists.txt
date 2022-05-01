
# prep for cpputest based build
set(TEST_APP_NAME HwLockCtrlServiceTests)

include_directories(${DRIVERS_TOP_DIR}/hwLockCtrl/include)

#note: we are building and linking with the MOCK LockCtrl module, instead
#      of the actual LockCtrl driver. We must also pull in
#      sources for cpputest execution and QP and our fake port of QP
set(TEST_SOURCES
        hwLockCtrlServiceTests.cpp
        ../src/hwLockCtrlService.cpp
        ${MOCKS_TOP_DIR}/hwLockCtrl/mockHwLockCtrl.cpp)

# this include expects TEST_SOURCES and TEST_APP_NAME to be
# defined, and creates the cpputest based test executable target
include(${TEST_SUPPORT_TOP_DIR}/common/cpputestCMake.txt)

target_link_libraries(${TEST_APP_NAME} cpputestqpcpplib ${CPPUTEST_LDFLAGS})