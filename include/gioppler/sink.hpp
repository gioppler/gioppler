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
#ifndef GIOPPLER_SINK_HPP
#define GIOPPLER_SINK_HPP

#if __cplusplus < 202002L
#error C++20 or newer support required to use this library.
#endif

#include <filesystem>
#include <future>
#include <functional>
#include <list>
#include <memory>
#include <random>
#include <unordered_map>
#include <variant>
#include <vector>
using namespace std::chrono_literals;

#include "gioppler/linux/config.hpp"
#include "gioppler/utility.hpp"

// -----------------------------------------------------------------------------
namespace gioppler::sink {

// -----------------------------------------------------------------------------
using Record = std::unordered_map<std::string, std::variant<bool, double, std::string>>;

// -----------------------------------------------------------------------------
class Sinks {
 public:
  Sinks() : _sinks{}, _workers{} { }

  ~Sinks() {
    const std::lock_guard<std::mutex> lock(_workers_mutex);
    wait_workers();
  }

  /// Add another sink to the chain.
  // Assumed to be called from only one thread at a time.
  void add_sink(std::function<bool(std::shared_ptr<Record>)> fn) {
    _sinks.emplace_back(fn);
  }

  /// Write the record to the sink.
  // Sink objects run in their own thread.
  void write_record(std::shared_ptr<Record> record) {
    const std::lock_guard<std::mutex> lock(_workers_mutex);
    check_workers();   // check before adding to avoid checking newly added workers
    for (auto&& sink : _sinks) {
      _workers.emplace_back(std::async(std::launch::async, sink, record));
    }
  }

 private:
  std::vector<std::function<bool(std::shared_ptr<Record>)>> _sinks;
  std::list<std::future<bool>> _workers;
  std::mutex _workers_mutex;

  /// Check and remove workers that have already finished executing.
  void check_workers() {
    _workers.remove_if([](std::future<bool>& fut)
      { return fut.wait_for(0s) == std::future_status::ready; });
  }

  /// Wait for the workers to finish and then delete them.
  void wait_workers() {
    _workers.remove_if([](std::future<bool>& fut)
      { fut.wait(); return true; });
  }
};

// -----------------------------------------------------------------------------
static inline Sinks g_sinks{};

// -----------------------------------------------------------------------------
/// log file destination using JSON format
// https://jsonlines.org/
// https://www.json.org/
class Json {
 public:
  static void add_sink() {

  }

  void write_record(std::shared_ptr<Record> record) {

  }

 private:
  Json() {
    create_filepath();
    std::clog << "INFO: setting gioppler log to " << _filepath << std::endl;
    _output_stream.open(_filepath, std::ios::trunc); // text mode
  }

  ~Json() {
    _output_stream.close();
  }

  void write_line(const std::string_view line) {
    _output_stream.write(line.data(), line.size());
  }

  std::once_flag _write_thread_init;
  std::string _filepath;
  std::ostream _output_stream;

  void create_filepath() {
    std::random_device random_device;
    std::independent_bits_engine<std::default_random_engine, 16, std::uint_least16_t>
      generator{random_device};
    const std::filesystem::path temp_path{std::filesystem::temp_directory_path()};
    const std::string program_name{get_program_name()};
    const uint64_t process_id{get_process_id()};
    const uint_least16_t salt{generator()};
    const std::string log_name{format("{}-{}-{}.json", program_name, process_id, salt)};
    _filepath = temp_path / log_name;
  }

  void init_write_thread() {

  }
};

// -----------------------------------------------------------------------------
}   // namespace gioppler::sink

// -----------------------------------------------------------------------------
#endif // defined GIOPPLER_SINK_HPP
