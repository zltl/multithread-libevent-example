
cmake_minimum_required(VERSION 3.9)

project(
  multithread-libevent-example
  VERSION 0.0.1
  LANGUAGES C CXX)

# It's 21 century!
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ImportLibrary)

# valgrind need
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-builtin-strlen")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-strlen")
# sanitize
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize-address-use-after-scope")


# download libevent
set(EVENT__LIBRARY_TYPE "STATIC")
import_library(
  libevent
  https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz
  b5333f021f880fe76490d8a799cd79f4)
add_subdirectory(${IMPORT_SRC} ${IMPORT_BUILD} EXCLUDE_FROM_ALL)

# download google test
import_library(
  googletest
  https://github.com/google/googletest/archive/refs/tags/release-1.10.0.zip
  82358affdd7ab94854c8ee73a180fc53)
add_subdirectory(${IMPORT_SRC} ${IMPORT_BUILD} EXCLUDE_FROM_ALL)

# add target
set(EXEC_NAME multithread-libevent-example)
add_executable(${EXEC_NAME}
  main.cc
  buffer.cc
  buffer.h
  thread_pool.h
  thread_pool.cc
  dispatcher.h
  dispatcher.cc)
# dependency libraries
target_link_libraries(${EXEC_NAME} event_core event_pthreads event_extra)
# Be regorous
target_compile_options(${EXEC_NAME} PUBLIC -Werror -Wall -Wextra -pedantic)

# test
add_subdirectory(tests)
