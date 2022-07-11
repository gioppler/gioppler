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
#ifndef GIOPPLER_GIOPPLER_HPP
#define GIOPPLER_GIOPPLER_HPP

#if __cplusplus < 202002L
#error C++20 or newer support required to use this header-only library.
#endif

#include <atomic>
#include <chrono>
#include <deque>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <variant>
#include <version>

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>

// -----------------------------------------------------------------------------
namespace gioppler {

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
class Program {
 public:
  Program() {
    _start = std::chrono::steady_clock::now();
  }

  ~Program() {
    const std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end - _start;
    _duration_secs = diff.count();
  }

  static void check_create() {
    std::call_once(_once_flag_create, call_once_create);
  }

  static void check_destroy() {
    std::call_once(_once_flag_destroy, call_once_destroy);
  }

 private:
  static inline std::once_flag _once_flag_create, _once_flag_destroy;
  static inline Program *_p_program;
  std::chrono::time_point<std::chrono::steady_clock> _start;
  static double _duration_secs;   // program duration

  static void call_once_create() {
    _p_program = new Program;
  }

  static void call_once_destroy() {
    delete _p_program;
    _p_program = nullptr;
  }
};

// ---------------------------------------------------------------------------
class Thread {
 public:
  explicit Thread() {
    const uint_fast64_t prev_threads_created = std::atomic_fetch_add(&_threads_created, 1);
    _thread_id = prev_threads_created + 1;
    std::atomic_fetch_add(&_threads_active, 1);
  }

  ~Thread() {
    std::atomic_fetch_sub(&_threads_active, 1);
  }

  static void check_create() {
    if (_p_thread == nullptr) {
      _p_thread = new Thread;
    }
  }

  static void destroy() {
    delete _p_thread;
    _p_thread = nullptr;
  }

  static bool all_threads_done() {
    return _threads_active == 0;
  }

  uint64_t get_id() {
    return _thread_id;
  }

 private:
  static thread_local inline Thread
  *
  _p_thread;
  static inline std::atomic_uint_fast64_t _threads_created;
  static inline std::atomic_uint_fast64_t _threads_active;
  uint_fast64_t _thread_id;
  LinuxEvents _linux_events;
};

// ---------------------------------------------------------------------------
class ProfileData {
 public:
  explicit ProfileData(const std::string_view parent_function_signature,
                       const std::string_view function_signature)
      :
      _parent_function_signature(parent_function_signature),
      _function_signature(function_signature),
      _function_calls(),
      _sum_of_count(),
      _linux_event_data_total(),
      _linux_event_data_self() {
  }

  static void write_header(std::ostream &os) {
    os << "Subsystem,ParentFunction,Function,Calls,Count,";
    LinuxEventsData::write_header(os);
  }

  void write_data(std::ostream &os) {

  }

  ProfileData &operator+=(const ProfileData &rhs) {
    _sum_of_count += rhs._sum_of_count;
    _linux_event_data_total += rhs._linux_event_data_total;
    _linux_event_data_self += rhs._linux_event_data_self;
    return *this;
  }

  friend ProfileData operator+(ProfileData lhs, const ProfileData &rhs) {
    lhs += rhs;
    return lhs;
  }

  ProfileData &operator-=(const ProfileData &rhs) {
    _sum_of_count -= rhs._sum_of_count;
    _linux_event_data_total -= rhs._linux_event_data_total;
    _linux_event_data_self -= rhs._linux_event_data_self;
    return *this;
  }

  friend ProfileData operator-(ProfileData lhs, const ProfileData &rhs) {
    lhs -= rhs;
    return lhs;
  }

 private:
  const std::string_view _parent_function_signature;
  const std::string_view _function_signature;
  uint64_t _function_calls;
  double _sum_of_count;
  LinuxEventsData _linux_event_data_total;
  LinuxEventsData _linux_event_data_self;
};

// ---------------------------------------------------------------------------
template<BuildMode build_mode = g_build_mode>
class Function {
 public:
  Function([[maybe_unused]] const std::string_view subsystem = "",
           [[maybe_unused]] const double count = 0.0,
           [[maybe_unused]] std::string session = "",
           [[maybe_unused]] const std::source_location &location =
           std::source_location::current())
  requires (build_mode == BuildMode::off) {
  }

  Function(const std::string_view subsystem = "",
           const double count = 0.0,
           std::string session = "",
           const std::source_location &location = std::source_location::current())
  requires (build_mode == BuildMode::profile) {
    check_create_program_thread();
  }

  ~Function() {
    check_destroy_program_thread();
  }

 private:
  using ProfileKey = std::pair<std::string_view, std::string_view>;
  static std::unordered_map<ProfileKey, ProfileData> _profile_map;
  static std::mutex _map_mutex;

  static thread_local inline std::stack<Function<build_mode>>  _functions;
  static thread_local inline std::stack<std::string>  _subsystems;
  static thread_local inline std::stack<std::string>  _sessions;

  // all accesses require modifying data
  // no advantage to use a readers-writer lock (a.k.a. shared_mutex)
  static void upsert_profile_map(const ProfileData &profile_record) {

  }

  void write_profile_map() {
    // sort descending by key
    std::multimap<double, ProfileKey, std::greater<>> sorted_profiles;
    sorted_profiles.reserve(_profile_map.size());

    // or sorted_profiles.copy(_profile_map.keys())
    //    sorted_profiles.sort()

    for (const auto &profile : _profile_map) {
      sorted_profiles.push_back(profile.first);
    }

    for (const auto &profile : sorted_profiles) {
      // std::cout << _profile_map[profile] << ' ';
    }
  }

  void check_create_program_thread() {
    Program::check_create();
    Thread::check_create();
  }

  void check_destroy_program_thread() {
    if (_functions.empty()) {
      Thread::destroy();
    }
    if (Thread::all_threads_done()) {
      write_profile_map();
      Program::check_destroy();
    }
  }
};

}   // namespace gioppler

#endif // defined GIOPPLER_GIOPPLER_HPP

// -----------------------------------------------------------------------------
int test(const int instance) {
  brainyguy::Function _{"test", 123, "hello"};
  std::cerr << "inside test " << instance << std::endl;
  if (instance > 1) {
    test(instance - 1);
  }
  return 0;
}

// -----------------------------------------------------------------------------
int main() {
  brainyguy::Function f;

  //const int t1_result = test();
  std::thread t2 = std::thread(test, 1);
  std::thread t3 = std::thread(test, 2);
  std::thread t4 = std::thread(test, 3);
  t2.join();
  t3.join();
  t4.join();
}
