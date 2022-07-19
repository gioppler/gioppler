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
#ifndef GIOPPLER_RECORD_HPP
#define GIOPPLER_RECORD_HPP

#if __cplusplus < 202002L
#error C++20 or newer support required to use this library.
#endif

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <source_location>
#include <syncstream>
#include <unordered_map>
#include <variant>
using namespace std::literals;

#include "gioppler/config.hpp"
#include "gioppler/platform.hpp"

// -----------------------------------------------------------------------------
namespace gioppler {

// -----------------------------------------------------------------------------
// Data Dictionary:
// - core.process.name      - string    - system process name
// - core.process.id        - integer   - system process id
// - core.thread.id         - integer   - system thread id
// - core.timestamp         - timestamp - when event occurred
// - core.build_mode        - string    - dev, test, prof, qa, prod
// - core.event             - string    - depends on build_mode
// - core.subsystem         - string    - user-supplied subsystem name
// - core.client            - string    - user-supplied client id
// - core.request           - string    - user-supplied request id
// - core.file              - string    - source file name and path
// - core.line              - int       - line number
// - core.column            - int       - column number
// - core.function          - string    - function name and signature
// - core.parent_function   - string    - parent (calling) function name and signature

// Linux Performance Counters: (category=profile)
// - prof.count             - integer   - number of times the function was called
// - prof.workload          - real      - user-assigned weight to profiled function calls

// All of these have two versions:
// - *.total                - sum of the function and other functions it calls
// - *.self                 - sum of only the function, excluding other functions called

// - prof.sw.duration            - real      - real (wall clock) duration (secs)
// - prof.sw.cpu_clock           - real      - CPU clock, a high-resolution per-CPU timer. (secs)
// - prof.sw.task_clock          - real      - clock count specific to the task that is running. (secs)
// - prof.sw.page_faults         - integer   - number of page faults
// - prof.sw.context_switches    - integer   - counts context switches
// - prof.sw.cpu_migrations      - integer   - number of times the process has migrated to a new CPU.
// - prof.sw.page_faults_min     - integer   - number of minor page faults.
// - prof.sw.page_faults_maj     - integer   - number of major page faults. These required disk I/O to handle.
// - prof.sw.alignment_faults    - integer   - counts the number of alignment faults. Zero on x86.
// - prof.sw.emulation_faults    - integer   - counts the number of emulation faults.

// - prof.hw.cpu_cycles          - integer   - Total cycles
// - prof.hw.instructions        - integer   - Retired instructions (i.e., executed)
// - prof.hw.stall_cycles_front  - integer   - Stalled cycles during issue in the frontend
// - prof.hw.stall_cycles_back   - integer   - Stalled cycles during retirement in the backend

// - prof.hw.cache_references    - integer   - Cache accesses.  Usually this indicates Last Level Cache accesses.
// - prof.hw.cache_misses        - integer   - Cache misses.  Usually this indicates Last Level Cache misses.

// - prof.hw.branch_instructions - integer   - Retired branch instructions (i.e., executed)
// - prof.hw.branch_misses       - integer   - Mispredicted branch instructions.

// -----------------------------------------------------------------------------
/// replacement for std::variant; eventually will be a wrapper
// std::variant is currently broken
// Error: Implicitly defined constructor deleted due to variant member
// https://en.cppreference.com/w/cpp/utility/variant
// https://stackoverflow.com/questions/53310690/implicitly-defined-constructor-deleted-due-to-variant-member-n3690-n4140-vs-n46
// https://github.com/llvm/llvm-project/issues/39034
// https://stackoverflow.com/questions/65404305/why-is-the-defaulted-default-constructor-deleted-for-a-union-or-union-like-class
// https://stackoverflow.com/questions/63624014/why-does-clang-using-libstdc-delete-the-explicitly-defaulted-constructor-on
class RecordValue
{
 public:
  enum class Type {Bool, Int, Real, String, Timestamp};

  Type get_type() const {
    return _record_value_type;
  }

  // ---------------------------------------------------------------------------
  RecordValue(const bool bool_value)
  : _record_value_type(Type::Bool), _bool_value(bool_value) { }

  [[nodiscard]] bool get_bool() const {
    assert(_record_value_type == Type::Bool);
    return _bool_value;
  }

  void set_bool(const bool bool_value) {
    assert(_record_value_type == Type::Bool);
    _bool_value = bool_value;
  }

  // ---------------------------------------------------------------------------
  RecordValue(const int64_t int_value)
  : _record_value_type(Type::Int), _int_value(int_value) { }

  RecordValue(const uint32_t int_value)
  : _record_value_type(Type::Int), _int_value(static_cast<int64_t>(int_value)) { }

  [[nodiscard]] int64_t get_int() const {
    assert(_record_value_type == Type::Int);
    return _int_value;
  }

  void set_int(const int64_t int_value) {
    assert(_record_value_type == Type::Int);
    _int_value = int_value;
  }

  // ---------------------------------------------------------------------------
  RecordValue(const double real_value)
  : _record_value_type(Type::Real), _real_value(real_value) { }

  [[nodiscard]] double get_real() const {
    assert(_record_value_type == Type::Real);
    return _real_value;
  }

  void set_real(const double real_value) {
    assert(_record_value_type == Type::Real);
    _real_value = real_value;
  }

  // ---------------------------------------------------------------------------
  RecordValue(std::string_view string_value)
  : _record_value_type(Type::String), _string_value(string_value) { }

  RecordValue(std::string string_value)
  : _record_value_type(Type::String), _string_value(std::move(string_value)) { }

  [[nodiscard]] std::string get_string() const {
    assert(_record_value_type == Type::String);
    return _string_value;
  }

  void set_string(std::string_view string_value) {
    assert(_record_value_type == Type::String);
    _string_value = string_value;
  }

  // ---------------------------------------------------------------------------
  RecordValue(Timestamp timestamp_value)
  : _record_value_type(Type::Timestamp), _timestamp_value(timestamp_value) { }

  [[nodiscard]] Timestamp get_timestamp() const {
    assert(_record_value_type == Type::Timestamp);
    return _timestamp_value;
  }

  void set_timestamp(const Timestamp timestamp_value) {
    assert(_record_value_type == Type::Timestamp);
    _timestamp_value = timestamp_value;
  }

 private:
  Type _record_value_type;
  bool _bool_value{};
  int64_t _int_value{};
  double _real_value{};
  std::string _string_value{};
  Timestamp _timestamp_value{};
};

// -----------------------------------------------------------------------------
/// Data being sent to a sink for processing.
using Record = std::unordered_map<std::string, RecordValue>;

// -----------------------------------------------------------------------------
/// convert a source_location into a string
// this is typically merged with other record maps
Record source_location_to_record(const std::source_location &location)
{
  return Record{
      {"file", location.file_name()},
      {"line", location.line()},
      {"column", location.column()},
      {"function", location.function_name()}
  };
}

// -----------------------------------------------------------------------------
}   // namespace gioppler

// -----------------------------------------------------------------------------
#endif // defined GIOPPLER_RECORD_HPP
