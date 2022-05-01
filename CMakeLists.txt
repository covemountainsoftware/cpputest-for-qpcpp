cmake_minimum_required(VERSION 3.16)
project(cpputest-for-qpcpp)

set(CMAKE_CXX_STANDARD 14)

add_compile_options(-Wall -Wextra -Werror)

include_directories(include)
include_directories(utilities/include)
include(externals/qpcppCMakeSupport.txt)

set(TEST_SUPPORT_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test_support)
set(MOCKS_TOP_DIR ${TEST_SUPPORT_TOP_DIR}/mocks)
set(SERVICES_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/services)
set(EXTERNALS_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)
set(DRIVERS_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/drivers)

add_subdirectory(utilities)
add_subdirectory(test_support)
add_subdirectory(drivers)
add_subdirectory(examples)
