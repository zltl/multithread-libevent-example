
enable_testing()

include_directories("${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")

macro(tl_add_test TESTNAME)
  add_executable(${TESTNAME} ${ARGN})
  target_compile_options(${TESTNAME} PUBLIC -Werror -Wall -Wextra -pedantic)
  target_link_libraries(
    ${TESTNAME}
    gtest
    gtest_main
    gmock
    gmock_main
    )
endmacro()

tl_add_test(buffer_test buffer_test.cc
  "${CMAKE_CURRENT_SOURCE_DIR}/../buffer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../buffer.cc")
