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
#ifndef GIOPPLER_UTILITY_HPP
#define GIOPPLER_UTILITY_HPP

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
/// String formatting function
#if defined(__cpp_lib_format)
// https://en.cppreference.com/w/cpp/utility/format
#include <format>
namespace gioppler {
template <typename... T>
[[nodiscard]] std::string format(std::string_view fmt, T&&... args) {
  return std::vformat(fmt, std::make_format_args(args...));
}
}   // namespace gioppler
#else
// https://github.com/fmtlib/fmt
#define FMT_HEADER_ONLY 1
#include <fmt/format.h>
#include <fmt/chrono.h>
namespace gioppler {
template <typename... T>
[[nodiscard]] std::string format(std::string_view fmt, T&&... args) {
  return fmt::vformat(fmt, fmt::make_format_args(args...));
}
}   // namespace gioppler
#endif

// -----------------------------------------------------------------------------
namespace gioppler {

// -----------------------------------------------------------------------------
// https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
template<typename T, typename... Rest>
void hash_combine(std::size_t &seed, const T &v, const Rest &... rest) {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hash_combine(seed, rest), ...);
}

struct pair_hash {
  template<class T1, class T2>
  std::size_t operator()(const std::pair<T1, T2> &pair) const {
    std::size_t seed = 0;
    hash_combine(seed, pair.first, pair.second);
    return seed;
  }
};

// -----------------------------------------------------------------------------
/// simplify creating shared pointers to unordered maps
// forces the use of an initialization list constructor
// without this we get an ambiguous overload error
// example: auto foo = make_shared_init_list<std::map<double,std::string>>({{1000, "hi"}});
// https://stackoverflow.com/questions/36445642/initialising-stdshared-ptrstdmap-using-braced-init
// https://stackoverflow.com/a/36446143/4560224
// https://en.cppreference.com/w/cpp/utility/initializer_list
// https://en.cppreference.com/w/cpp/language/list_initialization
template<class Container>
std::shared_ptr<Container>
make_shared_init_list(std::initializer_list<typename Container::value_type> il) {
    return std::make_shared<Container>(il);
}

// -----------------------------------------------------------------------------
/// Use the environment to resolve the location of the home directory.
std::filesystem::path get_home_path() {
  if (std::getenv("HOME")) {
    return std::getenv("HOME");
  } else if (std::getenv("HOMEDRIVE") && std::getenv("HOMEPATH")) {
    return std::filesystem::path(std::getenv("HOMEDRIVE")) += std::getenv("HOMEPATH");
  } else if (std::getenv("USERPROFILE")) {
    return std::getenv("USERPROFILE");
  } else {
    return "";
  }
}

// -----------------------------------------------------------------------------
/// Resolve macros, canonicalize, and create directory.
std::filesystem::path resolve_directory(const std::string_view directory) {
  std::filesystem::path directory_path;
  std::string_view rest;
  if (directory.starts_with("<temp>")) {
    rest = directory.substr("<temp>"sv.size());
    directory_path = std::filesystem::temp_directory_path();
  } else if (directory.starts_with("<home>")) {
    rest = directory.substr("<home>"sv.size());
    directory_path = get_home_path();
  } else if (directory.starts_with("<current>")) {
    rest = directory.substr("<current>"sv.size());
    directory_path = std::filesystem::current_path();
  } else if (directory.empty()) {
    directory_path = std::filesystem::current_path();
  } else {   // otherwise use path as is
    rest = directory;
  }

  directory_path += rest;
  directory_path = std::filesystem::weakly_canonical(directory_path);
  std::filesystem::create_directories(directory_path);
  return directory_path;
}

// -----------------------------------------------------------------------------
/// Create file path for sink destination.
std::filesystem::path create_filename(const std::string_view extension = "txt") {
  std::random_device random_device;
  std::independent_bits_engine<std::default_random_engine, 32, std::uint_least32_t>
    generator{random_device()};

  const std::string program_name{get_program_name()};
  const uint64_t process_id{get_process_id()};
  const uint32_t salt{generator() % 10'000};   // up to four digits
  const std::string extension_dot = (!extension.empty() && extension[0] != '.') ? "." : "";
  const std::string filename{format("{}-{}-{}{}{}}", program_name, process_id, salt, extension_dot, extension)};
  return filename;
}

// -----------------------------------------------------------------------------
/// Returns an open output stream for the given path and file extension.
// The stream does not require synchronization to use.
// https://en.cppreference.com/w/cpp/io/basic_ios/rdbuf
// https://en.cppreference.com/w/cpp/io/basic_ostream/basic_ostream
// Directory patterns:
//   <temp>, <current>, <home>   - optionally follow these with other directories
//   <cout>, <clog>, <cerr>      - these specify the entire path
std::unique_ptr<std::ostream>
get_output_filepath(const std::string_view directory = "<temp>"sv, const std::string_view extension = "txt"sv)
{
  if (directory == "<cerr>") {
    return std::make_unique<std::osyncstream>(std::cerr);
  } else if (directory == "<cout>") {
    return std::make_unique<std::osyncstream>(std::cout);
  } else if (directory == "<clog>") {
    return std::make_unique<std::osyncstream>(std::clog);
  }

  const std::filesystem::path directory_path = resolve_directory(directory);
  const std::filesystem::path filename_path  = create_filename(extension);
  const std::filesystem::path full_path      = directory_path/filename_path;
  std::clog << "INFO: setting gioppler log to " << full_path << std::endl;
  return std::make_unique<std::ofstream>(full_path, std::ios::trunc);
}

// -----------------------------------------------------------------------------
/// Convert a time point into ISO-8601 string format.
// https://en.wikipedia.org/wiki/ISO_8601
// https://www.iso.org/obp/ui/#iso:std:iso:8601:-1:ed-1:v1:en
// https://www.iso.org/obp/ui/#iso:std:iso:8601:-2:ed-1:v1:en
// https://en.cppreference.com/w/cpp/chrono/system_clock/formatter
// https://en.cppreference.com/w/cpp/chrono/utc_clock/formatter
// Note: C++20 utc_clock is not quite implemented yet for gcc.
// Parameter example: const auto start = std::chrono::system_clock::now();
std::string format_timestamp(const std::chrono::system_clock::time_point ts)
{
  const std::uint64_t timestamp_ns =
    std::chrono::duration_cast<std::chrono::nanoseconds>(ts.time_since_epoch()).count();
  const std::uint64_t ns = timestamp_ns % 1000'000'000l;
  return format("{0:%FT%T}.{1:09d}{0:%zZ}", ts, ns);
}

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
  enum class RecordValueType {Boolean, Integer, Real, String, Timestamp};

  // ---------------------------------------------------------------------------
  RecordValue(const bool bool_value)
  : _record_value_type(RecordValueType::Boolean), _bool_value(bool_value) { }

  [[nodiscard]] bool get_bool() const {
    assert(_record_value_type == RecordValueType::Boolean);
    return _bool_value;
  }

  void set_bool(const bool bool_value) {
    assert(_record_value_type == RecordValueType::Boolean);
    _bool_value = bool_value;
  }

  // ---------------------------------------------------------------------------
  RecordValue(const int64_t int_value)
  : _record_value_type(RecordValueType::Integer), _int_value(int_value) { }

  [[nodiscard]] int64_t get_int() const {
    assert(_record_value_type == RecordValueType::Integer);
    return _int_value;
  }

  void set_int(const int64_t int_value) {
    assert(_record_value_type == RecordValueType::Integer);
    _int_value = int_value;
  }

  // ---------------------------------------------------------------------------
  RecordValue(const double real_value)
  : _record_value_type(RecordValueType::Real), _real_value(real_value) { }

  [[nodiscard]] double get_real() const {
    assert(_record_value_type == RecordValueType::Real);
    return _real_value;
  }

  void set_real(const double real_value) {
    assert(_record_value_type == RecordValueType::Real);
    _real_value = real_value;
  }

  // ---------------------------------------------------------------------------
  RecordValue(std::string_view string_value)
  : _record_value_type(RecordValueType::String), _string_value(string_value) { }

  RecordValue(std::string string_value)
  : _record_value_type(RecordValueType::String), _string_value(std::move(string_value)) { }

  [[nodiscard]] std::string get_string() const {
    assert(_record_value_type == RecordValueType::String);
    return _string_value;
  }

  void set_string(std::string_view string_value) {
    assert(_record_value_type == RecordValueType::String);
    _string_value = string_value;
  }

  // ---------------------------------------------------------------------------
  RecordValue(std::chrono::system_clock::time_point timestamp_value)
  : _record_value_type(RecordValueType::Timestamp), _timestamp_value(timestamp_value) { }

  [[nodiscard]] std::chrono::system_clock::time_point get_timestamp() const {
    assert(_record_value_type == RecordValueType::Timestamp);
    return _timestamp_value;
  }

  void set_timestamp(const std::chrono::system_clock::time_point timestamp_value) {
    assert(_record_value_type == RecordValueType::Timestamp);
    _timestamp_value = timestamp_value;
  }

 private:
  RecordValueType _record_value_type;
  bool _bool_value{};
  int64_t _int_value{};
  double _real_value{};
  std::string _string_value{};
  std::chrono::system_clock::time_point _timestamp_value{};
};

// -----------------------------------------------------------------------------
/// Data being sent to a sink for processing.
using Record = std::unordered_map<std::string, RecordValue>;

// -----------------------------------------------------------------------------
/// convert a source_location into a string
std::string format_source_location(const std::source_location &location)
{
    std::string message =
      format("{}({}:{}): {}",
             location.file_name(),
             location.line(),
             location.column(),
             location.function_name());
    return message;
}

// -----------------------------------------------------------------------------
/// convert a source_location into a string
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
#endif // defined GIOPPLER_UTILITY_HPP
