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

#include <chrono>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>
using namespace std::chrono_literals;

#include "gioppler/config.hpp"
#include "gioppler/utility.hpp"
#include "gioppler/contract.hpp"

// -----------------------------------------------------------------------------
namespace gioppler::sink {

// the C++ standard library is not guaranteed to be thread safe
// file i/o operations are thread-safe on Windows and on POSIX systems
// the POSIX standard requires that C stdio FILE* operations are atomic
// https://docs.microsoft.com/en-us/cpp/standard-library/thread-safety-in-the-cpp-standard-library
// https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_concurrency.html
// https://en.cppreference.com/w/cpp/io/ios_base/sync_with_stdio
// Regarding cerr, cout, clog, etc:
//   Unless std::ios_base::sync_with_stdio(false) has been issued,
//   it is safe to concurrently access these objects from multiple threads for both formatted and unformatted output.
//   Due to the overhead necessitated by thread-safety, no other stream objects are thread-safe by default.

// -----------------------------------------------------------------------------
/// Data being sent to a sink for processing.
using Record = std::unordered_map<std::string,
  std::variant<bool, int64_t, double, std::string, std::chrono::system_clock::time_point>>;
enum class RecordIndex {boolean = 0, integer, real, string, timestamp};

// -----------------------------------------------------------------------------
struct Sink {
  virtual ~Sink() = default;
  virtual bool write_record(std::shared_ptr<Record> record) = 0;
};

// -----------------------------------------------------------------------------
/// Sink management class. Thread safe.
class SinkManager {
 public:
  SinkManager() : _sinks{}, _workers{} { }

  /// Waits for all sinks to finish processing data before exiting.
  ~SinkManager() {
    const std::lock_guard<std::mutex> lock{_mutex};
    wait_workers();
  }

  /// Add another sink to the chain.
  void add_sink(std::unique_ptr<Sink> sink) {
    const std::lock_guard<std::mutex> lock{_mutex};
    _sinks.emplace_back(std::move(sink));
  }

  /// Write the record to the sink.
  // Sink objects run in their own thread.
  void write_record(std::shared_ptr<Record> record) {
    std::call_once(_create_sinks_once_flag, check_create_sinks);
    const std::lock_guard<std::mutex> lock{_mutex};
    check_workers();   // check before adding to avoid checking newly added workers
    for (auto&& sink : _sinks) {
      _workers.emplace_back(std::async(std::launch::async, [&sink, record]{return sink->write_record(record);}));
    }
  }

 private:
  /// Sink objects are not copied but are called from multiple threads, one for each worker.
  std::vector<std::unique_ptr<Sink>> _sinks;
  std::list<std::future<bool>> _workers;
  std::mutex _mutex;
  std::once_flag _create_sinks_once_flag;

  /// Create default sinks if write attempted and no sinks defined already
  static void check_create_sinks();

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
static inline SinkManager g_sink_manager{};

// -----------------------------------------------------------------------------
/// log file destination using JSON format
// https://jsonlines.org/
// https://www.json.org/
class Json : public Sink {
 public:
  Json(std::string_view filepath = "") {
    _output_stream = get_output_filepath(filepath, ".json");
  }

  ~Json() {
  }

  /// add a new JSON format data record sink
  // Directory patterns:
  //   <temp>, <current>, <home>   - optionally follow these with other directories
  //   <cout>, <clog>, <cerr>      - these specify the entire path
  static void add_sink(std::string_view path = "<current>"sv) {
    g_sink_manager.add_sink(std::make_unique<Json>(path));
  }

 private:
  std::unique_ptr<std::ostream> _output_stream;
  std::mutex _mutex;

  bool is_record_filtered(const Record& record) {
    return false;
  }

  bool write_record(std::shared_ptr<Record> record) {
    if (is_record_filtered(*record)) {
      return false;
    }

    std::stringstream buffer;

    bool first_field = true;
    for (const auto& [key, value] : *record) {
      if (first_field) {
        first_field = false;
      } else {
        buffer.put(',');
      }

      switch (RecordIndex(value.index())) {
        case RecordIndex::boolean: {
          buffer << format("\"{}\":{}", key, std::get<bool>(value));
          break;
        }

        case RecordIndex::integer: {
          buffer << format("\"{}\":{}", key, std::get<int64_t>(value));
          break;
        }

        case RecordIndex::real: {
          buffer << format("\"{}\":{}", key, std::get<double>(value));
          break;
        }

        case RecordIndex::string: {
          buffer << format("\"{}\":\"{}\"", key, std::get<std::string>(value));
          break;
        }

        case RecordIndex::timestamp: {
          buffer << format("\"{}\":\"{}\"", key, format_timestamp(std::get<std::chrono::system_clock::time_point>(value)));
          break;
        }

        default: contract::assert(false);
      }
    }

    _output_stream->write(buffer.view().data(), buffer.view().size());
    return true;
  }

  void write_line(const std::string_view line) {
    _output_stream->write(line.data(), line.size());
  }
};

// -----------------------------------------------------------------------------
/// Create default sinks if write attempted and no sinks defined already
// Defined here, so we can refer to the sink classes.
void SinkManager::check_create_sinks() {
  if (g_sink_manager._sinks.empty()) {
    Json::add_sink();
    // TODO: add other default sinks
  }
}

// -----------------------------------------------------------------------------
}   // namespace gioppler::sink

// -----------------------------------------------------------------------------
#endif // defined GIOPPLER_SINK_HPP
