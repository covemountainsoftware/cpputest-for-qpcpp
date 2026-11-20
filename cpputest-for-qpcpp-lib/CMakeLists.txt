set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Warnings are errors only when building this project directly. When this
# library is added to another project, a newer compiler's new warnings in
# this library's code must not break that project's build.
option(CMS_WARNINGS_AS_ERRORS
       "Treat warnings in cpputest-for-qpcpp-lib code as errors"
       ${PROJECT_IS_TOP_LEVEL})

# Warning flags for this project's own code only, never third-party code.
set(CMS_WARNING_FLAGS -Wall -Wextra -Wpedantic -Wold-style-cast -Wsign-conversion)
if(CMS_WARNINGS_AS_ERRORS)
    list(APPEND CMS_WARNING_FLAGS -Werror)
endif()
# GCC/Clang style flags; wrap each flag so other compilers ignore them
list(TRANSFORM CMS_WARNING_FLAGS
     PREPEND "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:")
list(TRANSFORM CMS_WARNING_FLAGS APPEND ">")

include(${CMS_CMAKE_DIR}/cpputestFind.cmake)

# QP/C++ framework sources (third-party): built without this project's
# warning flags, and its headers are SYSTEM headers for all consumers.
add_library(cms-qpcpp ${CMS_QPCPP_QF_SRCS})
target_include_directories(cms-qpcpp SYSTEM
        PUBLIC ${CMS_QPCPP_INCLUDE_DIR}
        PRIVATE ${CMS_QPCPP_SRC_DIR})
# the cpputest QP/C++ port header (qp_port.hpp) lives in this library
target_include_directories(cms-qpcpp PUBLIC include)

add_library(cpputest-for-qpcpp-lib
        src/cpputest_qf_port.cpp
        src/cms_cpputest_qf_ctrl.cpp
        src/cms_cpputest_q_onAssert.cpp
        src/cms_cpputest_qf_onCleanup.cpp
        src/cpputestMain.cpp)

target_include_directories(cpputest-for-qpcpp-lib PUBLIC include)
# QP internal headers (qp_pkg.hpp), needed by the port implementation
target_include_directories(cpputest-for-qpcpp-lib SYSTEM PRIVATE ${CMS_QPCPP_SRC_DIR})
target_link_libraries(cpputest-for-qpcpp-lib PUBLIC
        qassert-meta-lib cms-qpcpp cms::cpputest)
target_compile_definitions(cpputest-for-qpcpp-lib PRIVATE
        CPPUTEST_FOR_QPCPP_LIB_VERSION="${cpputest-for-qpcpp-lib_VERSION}")
target_compile_options(cpputest-for-qpcpp-lib PRIVATE ${CMS_WARNING_FLAGS})

add_subdirectory(tests)
