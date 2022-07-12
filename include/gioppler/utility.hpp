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
#include <random>
#include <source_location>
#include "gioppler/config.hpp"

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
/// Create file path for sink destination.
std::string create_filepath(const std::string_view directory = ".", const std::string_view extension = ".txt") {
  std::filesystem::path directory_path;
  if (directory == "") {   // "" means use the system temporary file path
    directory_path = std::filesystem::temp_directory_path();
  } else if (directory == ".") {   // "." means use the current directory
    directory_path = std::filesystem::current_path();
  } else {   // otherwise path contains the directory to use for the log file
    directory_path = std::filesystem::canonical(std::filesystem::path(directory));
  }

  std::random_device random_device;
  std::independent_bits_engine<std::default_random_engine, 32, std::uint_least32_t>
    generator{random_device()};

  const std::string program_name{get_program_name()};
  const uint64_t process_id{get_process_id()};
  const uint32_t salt{generator() % 10'000};   // up to four digits
  const std::string log_name{format("{}-{}-{}{}}", program_name, process_id, salt, extension)};
  return directory_path / log_name;
}

// -----------------------------------------------------------------------------
}   // namespace gioppler

// -----------------------------------------------------------------------------
#endif // defined GIOPPLER_UTILITY_HPP
