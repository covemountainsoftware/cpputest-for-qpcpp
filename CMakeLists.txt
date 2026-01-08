cmake_minimum_required(VERSION 3.16)
project(cpputest-for-qpcpp-lib VERSION 1.6.0)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

set(CMS_EXTERNALS_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)
set(CMS_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cpputest-for-qpcpp-lib/cmake CACHE INTERNAL "")
set(CMS_QASSERT_META_TOP_DIR ${CMS_EXTERNALS_TOP_DIR}/qassert-meta)

if(NOT DEFINED CMS_QPCPP_TOP_DIR)
    set(CMS_QPCPP_TOP_DIR ${CMS_EXTERNALS_TOP_DIR}/qpcpp)
    FetchContent_Declare(qpcpp
            GIT_REPOSITORY https://github.com/QuantumLeaps/qpcpp.git
            GIT_TAG        1ed166adafcc7aca20ff6967f0dab89295c5c2d0 #8.1.2
            SOURCE_DIR     ${CMS_QPCPP_TOP_DIR}
    )
    message("Fetching QP/C++ git repository")
    FetchContent_MakeAvailable(qpcpp)
endif(NOT DEFINED CMS_QPCPP_TOP_DIR)

# enable the option to build qassert-meta-lib for QP/C++
option(CMS_ENABLE_QASSERT_META_QPCPP "Setup Internal QAssert Meta Data for QP/C++" ON)

FetchContent_Declare(qassert-meta
        GIT_REPOSITORY https://github.com/covemountainsoftware/qassert-meta.git
        GIT_TAG        ac2e18170208b78c797d8d2fc5374a9b2bf981ce
        SOURCE_DIR     ${CMS_QASSERT_META_TOP_DIR}
)
message("Fetching qassert-meta git repository")
FetchContent_MakeAvailable(qassert-meta)

include(${CMS_CMAKE_DIR}/qpcppCMakeSupport.cmake)
add_subdirectory(cpputest-for-qpcpp-lib)
