# Creates a CppUTest based test executable and runs it after each build.
#
# Expects:
#   TEST_APP_NAME - name of the test executable target to create
#   TEST_SOURCES  - the test source files
#   APP_LIB_NAME  - (optional) a library under test to link
#
# Provides CPPUTEST_LDFLAGS (cms::cpputest) for use with
# target_link_libraries(${TEST_APP_NAME} ...).
include(${CMAKE_CURRENT_LIST_DIR}/cpputestFind.cmake)

set(CPPUTEST_LDFLAGS cms::cpputest)

add_executable(${TEST_APP_NAME} ${TEST_SOURCES})
target_link_libraries(${TEST_APP_NAME} ${APP_LIB_NAME} ${CPPUTEST_LDFLAGS})

# Run the test once the build is done
add_custom_command(TARGET ${TEST_APP_NAME} POST_BUILD
                   COMMAND $<TARGET_FILE:${TEST_APP_NAME}>)
