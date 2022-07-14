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

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <source_location>
#include <syncstream>
#include "gioppler/config.hpp"
using namespace std::literals;

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
#define FMT_HEADER_ONLY
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
/// convert a source_location into a string
std::string format_source_location(const std::source_location &location)
{
    const std::string message =
      format("{}({}:{}): {}",
             location.file_name(),
             location.line(),
             location.column(),
             location.function_name());
    return message;
}

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
}   // namespace gioppler

// -----------------------------------------------------------------------------
#endif // defined GIOPPLER_UTILITY_HPP
