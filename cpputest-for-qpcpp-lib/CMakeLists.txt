include_directories(include)
include_directories(${CMS_QPCPP_SRC_DIR})
include_directories(${CMS_CPPUTEST_QP_PORT_TOP_DIR}/include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-DCPPUTEST_FOR_QPCPP_LIB_VERSION=\"${cpputest-for-qpcpp-lib_VERSION}\")

find_program(CPPCHECK cppcheck)
if (CPPCHECK)
    set(CMAKE_CXX_CPPCHECK
            ${CPPCHECK}
            --enable=warning,performance,portability
            --inline-suppr
            --quiet
            --suppress=missingIncludeSystem
            --error-exitcode=1
            -DUINTPTR_MAX=0xFFFFFFFF
    )
else ()
    message(WARNING "Cppcheck not found, install for additional analysis")
endif ()

add_library(cpputest-for-qpcpp-lib
        src/cpputest_qf_port.cpp
        src/cms_cpputest_qf_ctrl.cpp
        src/cms_cpputest_q_onAssert.cpp
        src/cms_cpputest_qf_onCleanup.cpp
        src/cpputestMain.cpp
        ${CMS_QPCPP_QF_SRCS})
target_link_libraries(cpputest-for-qpcpp-lib qassert-meta-lib)
add_subdirectory(tests)

target_include_directories(cpputest-for-qpcpp-lib PUBLIC
        ${CMS_QPCPP_INCLUDE_DIR}
        include)
