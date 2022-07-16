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
#ifndef GIOPPLER_COUNTER_HPP
#define GIOPPLER_COUNTER_HPP

#if __cplusplus < 202002L
#error C++20 or newer support required to use this library.
#endif

#include <memory>
#include <utility>

#include <gioppler/sink.hpp>

// -----------------------------------------------------------------------------
namespace gioppler {

// -----------------------------------------------------------------------------
struct CounterData {
  virtual ~CounterData() = default;
  virtual std::unique_ptr<CounterData> operator+=(std::unique_ptr<CounterData> rhs) = 0;

  friend std::unique_ptr<CounterData> operator+(std::unique_ptr<CounterData> lhs, std::unique_ptr<CounterData> rhs) {
    *lhs += std::move(rhs);
    return lhs;
  }

  virtual std::unique_ptr<sink::Record> create_record() = 0;
};

// -----------------------------------------------------------------------------
struct Counter {
  virtual ~Counter() = default;

  virtual void start() = 0;
  virtual void stop() = 0;

  virtual void enter_child() = 0;
  virtual void exit_child() = 0;

  virtual std::unique_ptr<CounterData> get_data() = 0;
};

// -----------------------------------------------------------------------------
struct CounterFactory {
  virtual ~CounterFactory() = default;
  virtual std::unique_ptr<Counter> create_counter() = 0;
};

// -----------------------------------------------------------------------------
}   // namespace gioppler

// -----------------------------------------------------------------------------
#if defined(GIOPPLER_PLATFORM_LINUX)
#include "gioppler/linux/counter.hpp"
#endif

// -----------------------------------------------------------------------------
#endif // defined GIOPPLER_COUNTER_HPP
