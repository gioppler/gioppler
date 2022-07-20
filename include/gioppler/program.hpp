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
#ifndef GIOPPLER_PROGRAM_HPP
#define GIOPPLER_PROGRAM_HPP

#if __cplusplus < 202002L
#error C++20 or newer support required to use this library.
#endif

#include <chrono>
#include <mutex>

#include "gioppler/utility.hpp"

// -----------------------------------------------------------------------------
namespace gioppler {

// -----------------------------------------------------------------------------
/// function to call on program exit
// https://en.cppreference.com/w/cpp/utility/program/exit
// https://en.cppreference.com/w/cpp/utility/program/quick_exit
// https://en.cppreference.com/w/cpp/error/terminate
// https://en.cppreference.com/w/cpp/utility/program/signal
extern void gioppler_exit();

// -----------------------------------------------------------------------------
class Program {
 public:
  Program() : _start{now()} { }

  ~Program() {
    const Timestamp end = now();
    const TimestampDiff diff = end - _start;
    double _duration_secs = diff.count();
    // ...
  }

 private:
  const Timestamp _start;
};

// -----------------------------------------------------------------------------
static inline Program g_program{};

// -----------------------------------------------------------------------------
}   // namespace gioppler

// -----------------------------------------------------------------------------
#endif // defined GIOPPLER_PROGRAM_HPP
