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

  virtual std::unique_ptr<Record> get_record() = 0;
};

// -----------------------------------------------------------------------------
class Counter {

  void enter_child() {

  }

  void exit_child() {

  }

  virtual std::unique_ptr<CounterData> get_data() = 0;
  virtual ~Counter() = 0;
};

// -----------------------------------------------------------------------------
std::unordered_map<std::string, uint64_t> counter_value;

// -----------------------------------------------------------------------------
class CounterFactory {
 public:
  CounterFactory() = default;
  virtual ~CounterFactory() = default;

 protected:
  virtual std::vector<uint64_t> get_counter_values() = 0;
  virtual std::vector<std::string> get_counter_names() = 0;
  virtual std::vector<double> get_counter_scales() = 0;

 private:

};

// -----------------------------------------------------------------------------
// defined in platform file:
// static inline thread_local std::unique_ptr<CounterFactory> g_counter_factory;

// -----------------------------------------------------------------------------
}   // namespace gioppler

// -----------------------------------------------------------------------------
#if defined(GIOPPLER_PLATFORM_LINUX)
#include "gioppler/linux/counter.hpp"
#endif

// -----------------------------------------------------------------------------
#endif // defined GIOPPLER_COUNTER_HPP
