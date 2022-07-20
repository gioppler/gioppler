#include <chrono>
#include <iostream>
#include <cstdint>

// https://github.com/fmtlib/fmt
#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fmt/chrono.h>
template <typename... T>
[[nodiscard]] std::string format(std::string_view fmt, T&&... args) {
  return fmt::vformat(fmt, fmt::make_format_args(args...));
}


// https://en.wikipedia.org/wiki/ISO_8601
// https://www.iso.org/obp/ui/#iso:std:iso:8601:-1:ed-1:v1:en
// https://www.iso.org/obp/ui/#iso:std:iso:8601:-2:ed-1:v1:en
// https://en.cppreference.com/w/cpp/chrono/system_clock/formatter
// https://en.cppreference.com/w/cpp/chrono/utc_clock/formatter
// Note: C++20 utc_clock is not quite implemented yet for gcc.
std::string
format_timestamp(const std::chrono::system_clock::time_point ts)
{
  const std::uint64_t timestamp_ns =
    std::chrono::duration_cast<std::chrono::nanoseconds>(ts.time_since_epoch()).count();
  const std::uint64_t ns = timestamp_ns % 1000'000'000l;
  return format("{0:%FT%T}.{1:09d}{0:%zZ}", ts, ns);
}

int main()
{
  const auto start = std::chrono::system_clock::now();
  std::cout << format_timestamp(start) << std::endl;
  return 0;
}
