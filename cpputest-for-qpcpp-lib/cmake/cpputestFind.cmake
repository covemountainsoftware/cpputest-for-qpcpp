# Locates the user's CppUTest and provides the interface target
# cms::cpputest (CppUTest + CppUTestExt). Linking to cms::cpputest
# provides CppUTest's include paths as SYSTEM includes (so this project's
# warning flags are not applied to CppUTest headers) and defines either
# CMS_CPPUTEST_LEGACY (CppUTest 3.x) or CMS_CPPUTEST_V4.
#
# Search order:
#   1. CppUTest/CppUTestExt targets already defined by a parent project
#      (e.g. via add_subdirectory or find_package of CppUTest)
#   2. The CPPUTEST_HOME environment variable (a CMake install of CppUTest,
#      or a tree with include/ and lib/)
#   3. pkg-config (cpputest >= 3.8), e.g. apt or Homebrew installs
#   4. find_package(CppUTest CONFIG), e.g. a CMAKE_PREFIX_PATH install
include_guard(GLOBAL)

set(_cms_cpputest_version "")
set(_cms_cpputest_libs "")

if(TARGET CppUTest AND TARGET CppUTestExt)
    message(STATUS "Using CppUTest targets provided by the parent project")
    set(_cms_cpputest_libs CppUTest CppUTestExt)
    if(DEFINED CppUTest_VERSION)
        set(_cms_cpputest_version ${CppUTest_VERSION})
    endif()
elseif(DEFINED ENV{CPPUTEST_HOME})
    set(_cms_cpputest_home $ENV{CPPUTEST_HOME})
    message(STATUS "Using CppUTest home: ${_cms_cpputest_home}")
    find_package(CppUTest CONFIG QUIET
                 PATHS ${_cms_cpputest_home} NO_DEFAULT_PATH)
    if(CppUTest_FOUND)
        set(_cms_cpputest_libs CppUTest CppUTestExt)
        set(_cms_cpputest_version ${CppUTest_VERSION})
    else()
        set(_cms_cpputest_lib_dirs
            ${_cms_cpputest_home}/lib
            ${_cms_cpputest_home}/lib64
            ${_cms_cpputest_home}/cpputest_build/lib)
        find_library(CMS_CPPUTEST_LIBRARY CppUTest
                     PATHS ${_cms_cpputest_lib_dirs} NO_DEFAULT_PATH REQUIRED)
        find_library(CMS_CPPUTEST_EXT_LIBRARY CppUTestExt
                     PATHS ${_cms_cpputest_lib_dirs} NO_DEFAULT_PATH REQUIRED)

        # imported targets: their include paths are SYSTEM for consumers
        add_library(cms_cpputest_core UNKNOWN IMPORTED)
        set_target_properties(cms_cpputest_core PROPERTIES
            IMPORTED_LOCATION ${CMS_CPPUTEST_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${_cms_cpputest_home}/include)
        add_library(cms_cpputest_ext UNKNOWN IMPORTED)
        set_target_properties(cms_cpputest_ext PROPERTIES
            IMPORTED_LOCATION ${CMS_CPPUTEST_EXT_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${_cms_cpputest_home}/include)
        # CppUTestExt depends upon CppUTest, so it is listed first
        set(_cms_cpputest_libs cms_cpputest_ext cms_cpputest_core)
    endif()
else()
    # pkg-config is tried before the CMake package: some distribution
    # packages (e.g. Ubuntu 24.04's CppUTest 4.0) ship a broken
    # CppUTestConfig.cmake that fails the configure step.
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_search_module(CPPUTEST QUIET IMPORTED_TARGET cpputest>=3.8)
    endif()
    if(CPPUTEST_FOUND)
        message(STATUS "Found CppUTest (pkg-config) version ${CPPUTEST_VERSION}")
        set(_cms_cpputest_libs PkgConfig::CPPUTEST)
        set(_cms_cpputest_version ${CPPUTEST_VERSION})
    else()
        find_package(CppUTest CONFIG REQUIRED)
        message(STATUS "Found CppUTest (CMake package) version ${CppUTest_VERSION}")
        set(_cms_cpputest_libs CppUTest CppUTestExt)
        set(_cms_cpputest_version ${CppUTest_VERSION})
    endif()
endif()

add_library(cms_cpputest INTERFACE)
add_library(cms::cpputest ALIAS cms_cpputest)
target_link_libraries(cms_cpputest INTERFACE ${_cms_cpputest_libs})

# Include paths of imported targets are already SYSTEM. CppUTest targets
# built from source by a parent project are not, so add them as SYSTEM here.
foreach(_cms_lib IN LISTS _cms_cpputest_libs)
    if(TARGET ${_cms_lib})
        get_target_property(_cms_imported ${_cms_lib} IMPORTED)
        if(NOT _cms_imported)
            target_include_directories(cms_cpputest SYSTEM INTERFACE
                $<TARGET_PROPERTY:${_cms_lib},INTERFACE_INCLUDE_DIRECTORIES>)
        endif()
    endif()
endforeach()

# An unknown version (e.g. a plain CPPUTEST_HOME tree) is treated as 4.x.
if(_cms_cpputest_version AND _cms_cpputest_version VERSION_LESS 4)
    target_compile_definitions(cms_cpputest INTERFACE CMS_CPPUTEST_LEGACY)
else()
    target_compile_definitions(cms_cpputest INTERFACE CMS_CPPUTEST_V4)
endif()
