# Copyright (c) 2023 Victor Chavez
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20.0)

project(zephyr_posix_demo)
set(BOARD native_posix_64)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

set(QPCPP_TEST_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
add_subdirectory(${QPCPP_TEST_DIR} ${CMAKE_BINARY_DIR}/cpputest)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}
										${QPCPP_TEST_DIR}/include
										${QPCPP_TEST_DIR}/utilities/include
										${QPCPP_TEST_DIR}/test_support/cpputest-qpcpp-port/include
										${QPCPP_TEST_DIR}/externals/qpcpp/include
										${QPCPP_TEST_DIR}/examples/hwLockCtrlService/include
)
target_sources(app PRIVATE	src/main.cpp
							sm/alarm.cpp
							mock/alarm_driver.cpp)


# find cpputest library
find_package(PkgConfig REQUIRED)
pkg_search_module(CPPUTEST REQUIRED cpputest>=3.8)  
target_link_libraries(app PRIVATE cpputestqpcpplib ${CPPUTEST_LDFLAGS})
