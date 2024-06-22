cmake_minimum_required(VERSION 3.16)
project(cpputest-for-qpcpp-lib VERSION 1.2.0)
include(FetchContent)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Wextra -Werror)

set(CMS_EXTERNALS_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)
set(CMS_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cpputest-for-qpcpp-lib/cmake CACHE INTERNAL "")

if(NOT DEFINED CMS_QPCPP_TOP_DIR)
    set(CMS_QPCPP_TOP_DIR ${CMS_EXTERNALS_TOP_DIR}/qpcpp)
    FetchContent_Declare(qpcpp
            GIT_REPOSITORY https://github.com/QuantumLeaps/qpcpp.git
            GIT_TAG        10441f7a03fa956b9e4772a073a05a96487983e2 #7.3.4
            SOURCE_DIR     ${CMS_QPCPP_TOP_DIR}
    )
    message("Fetching QP/C++ git repository")
    FetchContent_MakeAvailable(qpcpp)
endif(NOT DEFINED CMS_QPCPP_TOP_DIR)

include(${CMS_CMAKE_DIR}/qpcppCMakeSupport.cmake)
add_subdirectory(cpputest-for-qpcpp-lib)
