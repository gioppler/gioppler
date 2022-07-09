# Copyright (c) 2022 Carlos Reyes
# This code is licensed under the permissive MIT License (MIT).
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# versions supported by CLion 2022.1.3 are 2.8.11 to 3.22.x
# CMake 3.12 introduced support for C++20
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)            # set default for all targets

project(Gioppler
  VERSION 0.1.1
  DESCRIPTION "A profiling, logging, and testing library for C++20"
  HOMEPAGE_URL "https://github.com/gioppler/gioppler"
  LANGUAGES CXX)

include(CTest)

# ------------------------------------------------------------------------------
# http://mariobadr.com/creating-a-header-only-library-with-cmake.html
# https://dominikberner.ch/cmake-interface-lib/
# https://www.cppengineer.com/blog/using-cmake-to-create-header-only-shared-and-static-libraries
add_library(gioppler INTERFACE)
target_include_directories(gioppler INTERFACE "$<BUILD_INTERFACE:${Gioppler_SOURCE_DIR}/include>")

# Usage:
# target_link_libraries(your_app_or_lib PRIVATE gioppler)

# ------------------------------------------------------------------------------
file(GLOB TEST_SOURCE_LIST CONFIGURE_DEPENDS "${Gioppler_SOURCE_DIR}/test/*.cpp")
add_executable(gioppler_test "${TEST_SOURCE_LIST}")
target_include_directories(gioppler_test PUBLIC $<BUILD_INTERFACE:${Gioppler_SOURCE_DIR}/test>)
target_compile_options(gioppler_test PRIVATE -Werror -Wall)
target_link_libraries(gioppler_test PUBLIC m)
target_link_libraries(gioppler_test PRIVATE gioppler)

# ------------------------------------------------------------------------------
file(GLOB MAT_MULT_SOURCE_LIST CONFIGURE_DEPENDS "${Gioppler_SOURCE_DIR}/sample/matrix_mult.cpp")
add_executable(matrix_mult "${MAT_MULT_SOURCE_LIST}")
target_include_directories(matrix_mult PUBLIC $<BUILD_INTERFACE:${Gioppler_SOURCE_DIR}/sample>)
target_compile_options(matrix_mult PRIVATE -Werror -Wall)
target_link_libraries(matrix_mult PUBLIC m)
target_link_libraries(matrix_mult PRIVATE gioppler)

# ------------------------------------------------------------------------------
file(GLOB THREADS_SOURCE_LIST CONFIGURE_DEPENDS "${Gioppler_SOURCE_DIR}/sample/threads.cpp")
add_executable(threads "${THREADS_SOURCE_LIST}")
target_include_directories(threads PUBLIC $<BUILD_INTERFACE:${Gioppler_SOURCE_DIR}/sample>)
target_compile_options(threads PRIVATE -Werror -Wall)
target_link_libraries(threads PUBLIC m)
target_link_libraries(threads PRIVATE gioppler)

# ------------------------------------------------------------------------------
add_compile_definitions(PRIVATE GIOPPLER_BUILD_MODE_PROFILE=1)

# ------------------------------------------------------------------------------
if(BUILD_TESTING)
  add_test(NAME ackermann_ackermann4_1 COMMAND gioppler_test ackermann ackermann4_1)
endif()
