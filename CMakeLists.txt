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

cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)

project(Gioppler
  VERSION 0.1.1
  DESCRIPTION "A profiling, logging, and testing library for C++"
  LANGUAGES CXX C)

include(CTest)

# ------------------------------------------------------------------------------
file(GLOB LIB_HEADER_LIST CONFIGURE_DEPENDS "${Gioppler_SOURCE_DIR}/include/gioppler/*.hpp")
file(GLOB LIB_SOURCE_LIST CONFIGURE_DEPENDS "${Gioppler_SOURCE_DIR}/library/*.cpp")

add_library(gioppler SHARED ${LIB_HEADER_LIST} ${LIB_SOURCE_LIST})

# ------------------------------------------------------------------------------
target_include_directories(gioppler
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${Gioppler_SOURCE_DIR}/include>
)

target_compile_options(gioppler PRIVATE -Werror)
target_compile_features(gioppler PRIVATE cxx_std_20)
target_link_libraries(gioppler PUBLIC m)

# ------------------------------------------------------------------------------
file(GLOB TEST_SOURCE_LIST CONFIGURE_DEPENDS "${Gioppler_SOURCE_DIR}/test/*.cpp")
add_executable(gioppler_test "${TEST_SOURCE_LIST}")

target_include_directories(gioppler_test
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${Gioppler_SOURCE_DIR}/include>
)

target_link_libraries(gioppler_test PUBLIC gioppler)

# ------------------------------------------------------------------------------
file(GLOB MAT_MULT_C_SOURCE_LIST CONFIGURE_DEPENDS "${Gioppler_SOURCE_DIR}/sample/matrix_mult.c")
add_executable(matrix_mult_c "${MAT_MULT_C_SOURCE_LIST}")

target_include_directories(matrix_mult_c
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${Gioppler_SOURCE_DIR}/include>
)

target_link_libraries(matrix_mult_c PUBLIC gioppler)

# ------------------------------------------------------------------------------
file(GLOB THREADS_SOURCE_LIST CONFIGURE_DEPENDS "${Gioppler_SOURCE_DIR}/sample/threads.cpp")
add_executable(threads "${THREADS_SOURCE_LIST}")

target_include_directories(threads
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${Gioppler_SOURCE_DIR}/include>
)

target_compile_features(threads PRIVATE cxx_std_20)
target_link_libraries(threads PUBLIC gioppler)

# ------------------------------------------------------------------------------
add_compile_definitions(PRIVATE GIOPPLER_BUILD_MODE_PROFILE=1)

# ------------------------------------------------------------------------------
if(BUILD_TESTING)
  add_test(NAME ackermann_ackermann4_1 COMMAND gioppler_test ackermann ackermann4_1)
endif()
