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

#include <cassert>
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
  explicit Json(std::string_view filepath) {
    _output_stream = get_output_filepath(filepath, "json"sv);
  }

  ~Json() override = default;

  /// add a new JSON format data record sink
  // Directory patterns:
  //   <temp>, <current>, <home>   - optionally follow these with other directories
  //   <cout>, <clog>, <cerr>      - these specify the entire path
  static void add_sink(std::string_view path = "<current>"sv) {
    g_sink_manager.add_sink(std::make_unique<Json>(path));
  }

 protected:
  bool write_record(std::shared_ptr<Record> record) override {
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

      switch (value.get_type()) {
        case RecordValue::Type::Bool: {
          buffer << format("\"{}\":{}", key, value.get_bool());
          break;
        }

        case RecordValue::Type::Int: {
          buffer << format("\"{}\":{}", key, value.get_int());
          break;
        }

        case RecordValue::Type::Real: {
          buffer << format("\"{}\":{}", key, value.get_real());
          break;
        }

        case RecordValue::Type::String: {
          buffer << format("\"{}\":\"{}\"", key, value.get_string());
          break;
        }

        case RecordValue::Type::Timestamp: {
          buffer << format("\"{}\":\"{}\"", key, format_timestamp(value.get_timestamp()));
          break;
        }

        default: assert(false);
      }
    }

    _output_stream->write(buffer.view().data(), buffer.view().size());
    return true;
  }

 private:
  std::unique_ptr<std::ostream> _output_stream;
  std::mutex _mutex;

  bool is_record_filtered(const Record& record) {
    return false;
  }

  void write_line(const std::string_view line) {
    _output_stream->write(line.data(), line.size());
  }
};

// -----------------------------------------------------------------------------
/// log file destination using CSV format
class Csv : public Sink {
 public:
  explicit Csv(std::vector<std::string> fields, std::string_view filepath = "<current>"sv,
      std::string_view separator = ","sv, std::string_view string_quote_char = "\""sv)
  : _fields(std::move(fields)), _separator(separator), _string_quote_char(string_quote_char)
  {
    _output_stream = get_output_filepath(filepath, "txt");
  }

  ~Csv() override = default;

  /// add a new CSV format data record sink
  // Directory patterns:
  //   <temp>, <current>, <home>   - optionally follow these with other directories
  //   <cout>, <clog>, <cerr>      - these specify the entire path
  static void add_sink(std::vector<std::string> fields, std::string_view filepath = "<current>"sv,
    std::string_view separator = ","sv, std::string_view string_quote_char = "\""sv)
  {
    g_sink_manager.add_sink(std::make_unique<Csv>(fields, filepath, separator, string_quote_char));
  }

 protected:
  bool write_record(std::shared_ptr<Record> record) override {
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

      switch (value.get_type()) {
        case RecordValue::Type::Bool: {
          buffer << format("\"{}\":{}", key, value.get_bool());
          break;
        }

        case RecordValue::Type::Int: {
          buffer << format("\"{}\":{}", key, value.get_int());
          break;
        }

        case RecordValue::Type::Real: {
          buffer << format("\"{}\":{}", key, value.get_real());
          break;
        }

        case RecordValue::Type::String: {
          buffer << format("\"{}\":\"{}\"", key, value.get_string());
          break;
        }

        case RecordValue::Type::Timestamp: {
          buffer << format("\"{}\":\"{}\"", key, format_timestamp(value.get_timestamp()));
          break;
        }

        default: assert(false);
      }
    }

    _output_stream->write(buffer.view().data(), buffer.view().size());
    return true;
  }

 private:
  std::unique_ptr<std::ostream> _output_stream;
  std::mutex _mutex;
  std::vector<std::string> _fields;
  std::string_view _separator;
  std::string_view _string_quote_char;

  bool is_record_filtered(const Record& record) {
    return false;
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
