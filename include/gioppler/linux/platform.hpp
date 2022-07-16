// Copyright (c) 2022 Carlos Reyes
// This code is licensed under the permissive MIT License (MIT).
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#ifndef GIOPPLER_LINUX_PLATFORM_HPP
#define GIOPPLER_LINUX_PLATFORM_HPP

#if __cplusplus < 202002L
#error C++20 or newer support required to use this library.
#endif

#include <string>

// -----------------------------------------------------------------------------
/// Program name
#if defined(GIOPPLER_PLATFORM_LINUX)      // Linux kernel; could be GNU or Android
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <errno.h>
namespace gioppler {
std::string get_program_name()
{
  return program_invocation_short_name;
}
#else
namespace gioppler {
std::string get_program_name()
{
  return "unknown";
}
#endif

// -----------------------------------------------------------------------------
/// Process id
#if defined(GIOPPLER_PLATFORM_LINUX)      // Linux kernel; could be GNU or Android
#include <unistd.h>
uint64_t get_process_id()
{
  return getpid();
}
#else
uint64_t get_process_id()
{
  return 0;
}
#endif

// -----------------------------------------------------------------------------
}   // namespace gioppler

// -----------------------------------------------------------------------------
#endif // defined GIOPPLER_LINUX_PLATFORM_HPP
