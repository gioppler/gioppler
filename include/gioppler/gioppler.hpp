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
#include <source_location>
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
// -----------------------------------------------------------------------------
namespace gioppler {

// -----------------------------------------------------------------------------
// g_build_mode controls the operating mode for the library.
// Normally this is controlled from the CMake build files.
// Define one of these constants to manually control the variable.
enum class BuildMode {off, development, test, profile, qa, production};
#if defined(GIOPPLER_BUILD_MODE_OFF)
constexpr static inline BuildMode g_build_mode = BuildMode::off;
#elif defined(GIOPPLER_BUILD_MODE_DEVELOPMENT)
constexpr static inline BuildMode g_build_mode = BuildMode::development;
#elif defined(GIOPPLER_BUILD_MODE_TEST)
constexpr static inline BuildMode g_build_mode = BuildMode::test;
#elif defined(GIOPPLER_BUILD_MODE_PROFILE)
constexpr static inline BuildMode g_build_mode = BuildMode::profile;
#elif defined(GIOPPLER_BUILD_MODE_QA)
constexpr static inline BuildMode g_build_mode = BuildMode::qa;
#elif defined(GIOPPLER_BUILD_MODE_PRODUCTION)
constexpr static inline BuildMode g_build_mode = BuildMode::production;
#else
#warning Build mode not defined. Disabling Gioppler library.
constexpr static inline BuildMode g_build_mode = BuildMode::off;
#endif

// -----------------------------------------------------------------------------
/// Platform defines the operating system.
// Often used to control include files, so define constants also.
enum class Platform {linux, windows, bsd};
#if defined(__linux__) || defined(__ANDROID__)
#define GIOPPLER_PLATFORM_LINUX 1
constexpr static inline Platform g_platform = Platform::linux;
#elif defined(_WIN32) || defined(_WIN64)
#define GIOPPLER_PLATFORM_WINDOWS 1
constexpr static inline Platform g_platform = Platform::windows;
#elif defined(BSD) || defined(__FreeBSD__) || defined(__NetBSD__) || \
      defined(__OpenBSD__) || defined(__DragonFly__)
#define GIOPPLER_PLATFORM_BSD 1
constexpr static inline Platform g_platform = Platform::bsd;
#else
#error Operating system platform unsupported.
#endif

// -----------------------------------------------------------------------------
/// Major compilers. Sometimes used to enable features.
enum class Compiler {gcc, clang, msvc, unknown};
#if defined(__clang__)
constexpr static inline Compiler g_compiler = Compiler::clang;
#elif defined(__GNUC__)
constexpr static inline Compiler g_compiler = Compiler::gcc;
#elif defined(_MSC_VER)
constexpr static inline Compiler g_compiler = Compiler::msvc;
#else
#warning Could not identify C++ compiler.
constexpr static inline Compiler g_compiler = Compiler::unknown;
#endif

// -----------------------------------------------------------------------------
/// CPU architecture.
enum class Architecture {x86, arm, unknown};
#if defined(__i386__) || defined(__x86_64__)
constexpr static inline Architecture g_architecture = Architecture::x86;
#elif defined(__arm__) || defined(__arm64__) || defined(__aarch64__)
constexpr static inline Architecture g_architecture = Architecture::arm;
#else
#warning Could not identify CPU architecture.
constexpr static inline Architecture g_architecture = Architecture::unknown;
#endif

// -----------------------------------------------------------------------------
}   // namespace gioppler

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
/// Performance Monitoring Counters (pmc), a Linux feature.
#if defined(GIOPPLER_PLATFORM_LINUX)      // Linux kernel; could be GNU or Android
#include <linux/perf_event.h>             // Definition of PERF_* constants
#include <linux/hw_breakpoint.h>          // Definition of HW_* constants
#include <sys/syscall.h>                  // Definition of SYS_* constants
#include <sys/ioctl.h>
#include <unistd.h>
namespace gioppler {
constexpr static inline bool g_performance_counters = true;
}
#else
namespace gioppler {
constexpr static inline bool g_performance_counters = false;
}
#endif

// -----------------------------------------------------------------------------
/// Program name
#if defined(GIOPPLER_PLATFORM_LINUX)      // Linux kernel; could be GNU or Android
#define _GNU_SOURCE
#include <errno.h>
namespace gioppler {
std::string get_program_name()
{
  return program_invocation_short_name;
}
#else
namespace gioppler {
std::string get_program_name()
{
  return "gioppler";
}
#endif

// -----------------------------------------------------------------------------
/// Process id
#if defined(GIOPPLER_PLATFORM_LINUX)      // Linux kernel; could be GNU or Android
#include <unistd.h>
uint64_t get_process_id()
{
  return getpid();
}
#else
uint64_t get_process_id()
{
  return 0;
}
#endif

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
#include <fmt/core.h>
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

std::string format_timestamp(const ) {


}

// -----------------------------------------------------------------------------
// https://jsonlines.org/
// https://www.json.org/
class JsonLine {
  public:


  private:
    std::unordered_map<std::string, std::variant<bool, double, std::string>> _fields;
};


// -----------------------------------------------------------------------------
/// log file destination
class Sink {
 public:
  Sink() {
    create_filepath();
    std::clog << "INFO: setting gioppler log to " << _filepath << std::endl;
    _output_stream.open(_filepath, std::ios::trunc); // text mode
  }

  ~Sink() {
    _output_stream.close();
  }

  void write_line(const std::string_view line) {
    _output_stream.write(line.data(), line.size());
  }

 private:
  std::once_flag _write_thread_init;
  std::string _filepath;
  std::ostream _output_stream;

  void create_filepath() {
    std::random_device random_device;
    std::independent_bits_engine<std::default_random_engine, 16, std::uint_least16_t>
      generator{random_device};
    const std::filesystem::path temp_path{temp_directory_path()};
    const std::string program_name{get_program_name()};
    const uint64_t process_id{get_process_id()};
    const uint_least16_t salt{generator()};
    const std::string log_name{format("{}-{}-{}.json", program_name, process_id, salt)};
    _filepath = temp_path / log_name;
  }

  void init_write_thread() {

  }
}
};

static Sink g_sink = Sink();

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
/// Contracts to ensure correct program behavior.
// These print messages to cerr and throw exceptions, as needed.
// They do not try to write to the log, assuming it may not be set up.
// These functions will always succeed.
namespace contract {

// -----------------------------------------------------------------------------
/// a contract condition has been violated
//
// logic_error are errors that are a consequence of faulty logic
// within the program such as violating logical preconditions or
// class invariants and may be preventable.
class contract_violation : public logic_error
{
public:
    explicit contract_violation(std::string_view what_arg)
    : logic_error(what_arg) { }

    virtual ~contract_violation() noexcept { }

    contract_violation(const contract_violation&) = default;
    contract_violation(contract_violation&&) = default;
    contract_violation& operator=(const contract_violation&) = default;
    contract_violation& operator=(contract_violation&&) = default;
};

// -----------------------------------------------------------------------------
/// errors that arise because an argument value has not been accepted
// check expect condition but only print to std::cerr
void argument(const bool condition,
              [[maybe_unused]] const std::source_location &location =
                std::source_location::current())
{
  if (!condition) [[unlikely]] {
    const std::string message =
      format("ERROR: {}: invalid argument\n",
             format_source_location(location));
      std::cerr << message;
      throw contract_violation{message};
  }
}

// -----------------------------------------------------------------------------
/// expect conditions are like preconditions
// check expect condition but only print to std::cerr
void expect(const bool condition,
            [[maybe_unused]] const std::source_location &location =
              std::source_location::current())
{
  if (!condition) [[unlikely]] {
    const std::string message =
      format("ERROR: {}: expect condition failed\n",
             format_source_location(location));
      std::cerr << message;
      throw contract_violation{message};
  }
}

// -----------------------------------------------------------------------------
}   // namespace contract

// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
namespace histogram
{

/// observation index into histogram
using rank_t        = uint32_t;

/// the values we are tracking in the histogram
using observation_t = uint64_t;

// -----------------------------------------------------------------------------
/// bucket of observations for the histogram
// provides support for ranking (quantile) operations
// observations are uint64 values
// count value goes up to 16 million
// stores the minimum and maximum observation values for the bucket
// actually stores the minimum observation and a span (range) to save bits
// uses linear interpolation to estimate the observation for the requested rank
// Note: this is an internal class intended to be called only from Histogram
class Bucket
{
public:
  Bucket()
    : _observation_min{}, _observation_span{}, _count{}
  {
    assert(invariant());
  }

  explicit Bucket(const observation_t observation)
    : _observation_min{observation}, _observation_span{}, _count{1}
  {
    assert(invariant());
  }

  Bucket& operator +=(const Bucket& rhs)
  {
    assert(invariant());
    const observation_t min = std::min(get_min(), rhs.get_min());
    const observation_t max = std::max(get_max(), rhs.get_max());
    _observation_min  = min;
    _observation_span = max-min;
    _count += rhs._count;
    assert(invariant());
    return *this;
  }

  observation_t get_min() const {
    return _observation_min;
  }

  observation_t get_max() const {
    return _observation_min+_observation_span;
  }

  rank_t get_count() const {
    return static_cast<rank_t>(_count);
  }

  // https://stackoverflow.com/questions/325933/determine-whether-two-date-ranges-overlap/325964#325964
  bool is_overlapping(const Bucket& b) const {
    return (get_min() <= b.get_max() && get_max() >= b.get_min());
  }

  bool contains_rank(const rank_t rank) const {
    return (rank >= 1 && rank <= _count);
  }

  /// get the observation contained using the rank to interpolate within the bucket
  // 1 .. _count
  double get_rank(const rank_t rank) const
  {
    assert(invariant());
    assert(contains_rank(rank));
    if (_count == 1) {
      return _observation_min;
    } else {
      return _observation_min + (rank-1)*_observation_span/(_count-1);
    }
  }

  friend constexpr bool
  operator <(const Bucket& l, const Bucket& r) {
    return (l._observation_min < r._observation_min);
  }

  std::ostream&
  write(std::ostream& os) const {
    os << '[' << get_min() << ',' << get_count() << ',' << get_max() << ']';
    return os;
  }

private:
  observation_t _observation_min;       // low end of range containing within this bucket
  observation_t _observation_span :40;  // about 10^12 (78 hours in ns)
  observation_t _count :24;             // about 10^7  (16,777,216)

  bool invariant() const {              // not a lot we can validate here
    assert(_count > 0);
    return (_count > 0);
  }
};

std::ostream&
operator <<(std::ostream& os, const Bucket& bucket) {
  return bucket.write(os);
}

// -----------------------------------------------------------------------------
/// histogram of uint64 values
// provides robust statistics computed using quantiles
// uses a vector to store the buckets with periodic compacting
// the vector is not resized once initially created
// number of buckets can handle millions of observations
// buckets have varying widths and are only allocated as needed
// compacting aims the level the number of observations per bucket
// https://en.wikipedia.org/wiki/Histogram
// https://en.wikipedia.org/wiki/Robust_statistics
// https://en.wikipedia.org/wiki/Quantile
class Histogram
{
public:
  Histogram()
    : _observations{}, _compacted(true)
  {
    _buckets.reserve(max_buckets);
    assert(invariant());
  }

  void add_observation(const observation_t observation)
  {
    assert(invariant());
    _buckets.emplace_back(Bucket(observation));
    ++_observations;
    _compacted = false;
    assert(invariant());

    if (_buckets.size() >= max_buckets) {
      compact_buckets();
    }
  }

  rank_t get_count() const {
    return _observations;
  }

  // 1 .. _observations
  observation_t get_by_rank(const rank_t rank) const
  {
    if (!_observations)   return 0;

    compact_buckets();
    assert(rank >= 1 && rank <= _observations);
    rank_t delta_rank = std::clamp(rank, static_cast<rank_t>(1), _observations);

    for (const auto& bucket : _buckets) {
      if (bucket.contains_rank(delta_rank)) {
        return bucket.get_rank(delta_rank);
      } else {
        delta_rank -= bucket.get_count();
      }
    }

    assert(false);   // should never get here
  }

  /// average of Q1+2*median+Q3
  // https://en.wikipedia.org/wiki/Trimean
  // https://en.wikipedia.org/wiki/Location_parameter
  // https://en.wikipedia.org/wiki/Trimmed_estimator
  observation_t get_trimedian() const
  {
    compact_buckets();
    if (_observations < 4) {
      if (!_observations) {
        return 0;
      } else if (_observations <= 2) {
        return get_by_rank(1);
      } else {   // 3
        return get_by_rank(2);
      }
    }

    const rank_t q1_rank       = round_div(_observations, 4);
    const rank_t q2_rank       = round_div(_observations, 2);
    const rank_t q3_rank       = q1_rank+q2_rank;
    const observation_t q1_obs = get_by_rank(q1_rank);
    const observation_t q2_obs = get_by_rank(q2_rank);
    const observation_t q3_obs = get_by_rank(q3_rank);
    return round_div(q1_obs+2*q2_obs+q3_obs, 4);
  }

  /// delta between 25th and 75th quantiles
  // https://en.wikipedia.org/wiki/Interquartile_range
  observation_t get_interquantile_range() const
  {
    compact_buckets();
    if (_observations < 4) {
      if (_observations <= 1) {
        return 0;
      } else if (_observations == 2) {
        return get_by_rank(2)-get_by_rank(1);
      } else {   // 3
        return get_by_rank(3)-get_by_rank(1);
      }
    }

    const rank_t q1_rank       = round_div(_observations, 4);
    const rank_t q2_rank       = round_div(_observations, 2);
    const rank_t q3_rank       = q1_rank+q2_rank;
    const observation_t q1_obs = get_by_rank(q1_rank);
    const observation_t q3_obs = get_by_rank(q3_rank);
    return (q3_obs-q1_obs);
  }

  /// an indication on how much noise is in the observations
  // 0=much noise, 99=very little noise
  // https://en.wikipedia.org/wiki/Coefficient_of_variation
  // https://en.wikipedia.org/wiki/Efficiency_(statistics)
  // https://en.wikipedia.org/wiki/Signal-to-noise_ratio
  // https://en.wikipedia.org/wiki/Logarithmic_scale
  // 0 .. 99
  int get_signal_to_noise_ratio() const
  {
    double trimedian = get_trimedian();
    trimedian = (trimedian == 0.0) ? 1.0 : trimedian;
    double std_dev   = get_standard_deviation();
    std_dev = (std_dev < 1) ? 1.0 : std_dev;
    const double snr = std::clamp(10*std::log10((trimedian*trimedian)/(std_dev*std_dev)), 0.0, 99.0);
    return std::lround(snr);
  }

  /// Do we have any outliers below or above most of the observations?
  // An outlier is a value six sigma away from the median
  //   in excess of the expectation from the standard deviation
  //   if we assume a normal distribution.
  // alternative method:   https://en.wikipedia.org/wiki/Outlier#Tukey's_fences
  std::pair<bool,bool> have_outliers() const
  {
    // https://en.wikipedia.org/wiki/68%E2%80%9395%E2%80%9399.7_rule
    static constexpr double stddev_6sigma = (1-0.999999998026825)/2;   // proportion outside interval
    const rank_t expected_outliers = std::lround(get_count()*stddev_6sigma);

    compact_buckets();
    const double trimedian          = get_trimedian();
    const double standard_deviation = get_standard_deviation();

    const double low_6sigma = trimedian-6*standard_deviation;
    rank_t low_outliers = 0;
    if (low_6sigma >= get_min()) {
      const rank_t low_values = count_low_values(std::lround(low_6sigma));
      if (low_values > expected_outliers) {
        low_outliers = low_values-expected_outliers;
      }
    }

    const double high_6sigma = trimedian+6*standard_deviation;
    rank_t high_outliers = 0;
    if (high_6sigma <= get_max()) {
      const rank_t high_values = count_high_values(std::lround(high_6sigma));
      if (high_values > expected_outliers) {
        high_outliers = high_values-expected_outliers;
      }
    }

    return std::make_pair<>(low_outliers, high_outliers);
  }

  /// A sparkline is a textual representation of a simple chart.
  // need to use reinterpret_cast to get around poor support for UTF-8 in C++20
  std::string get_sparkline(const observation_t width = 9) const
  {
    if (!_buckets.size())   return "";

    compact_buckets();
    static const std::vector<std::u8string> steps = {u8"▁", u8"▂", u8"▃", u8"▄", u8"▅", u8"▆", u8"▇", u8"█"};

    std::string sparkline;
    std::vector<rank_t> buckets(width);
    const observation_t min_value    = get_min();
    const observation_t max_value    = get_max();
    const observation_t range_value  = max_value-min_value;
    const double        bucket_width = static_cast<double>(range_value) / (width-1);

    for (const auto& bucket : _buckets) {
      for (rank_t rank = 1; rank <= bucket.get_count(); ++rank) {
        const rank_t index = static_cast<rank_t>((bucket.get_rank(rank)-min_value) / bucket_width);
        assert(index >= 0 && index < width);
        ++buckets[index];
      }
    }

    rank_t max_height = 0;
    for (const auto& bucket : buckets) {
      if (bucket > max_height) {
        max_height = bucket;
      }
    }

    for (const auto& bucket : buckets) {
      const size_t step = round_div(static_cast<rank_t>(bucket * (steps.size()-1)), max_height);
      assert(step >= 0 && step < steps.size());
      sparkline += reinterpret_cast<const char*>(steps[step].c_str());
    }

    return sparkline;
  }

  std::string get_statistics() const
  {
    auto [have_low_outliers, have_high_outliers] = have_outliers();
    std::stringstream oss;
    oss << '{' << "min:" << get_min()
        << ',' << "max:" << get_max()
        << ',' << "count:" << get_count()
        << ',' << "low_outliers:" << std::boolalpha << have_low_outliers
        << ',' << "high_outliers:" << std::boolalpha << have_high_outliers
        << ',' << "trimedian:" << get_trimedian()
        << ',' << "std_dev:" << get_standard_deviation()
        << ',' << "snr:" << get_signal_to_noise_ratio()
        << ',' << "sparkline:" << get_sparkline()
        << '}';
    return oss.str();
  }

  std::ostream& write(std::ostream& os) const
  {
    compact_buckets();
    os << '[';
    for (const auto& bucket : _buckets) {
      os << bucket;
    }
    os << ']';
    return os;
  }

private:
  rank_t _observations;                       // count of observations
  mutable bool _compacted;
  mutable std::vector<Bucket> _buckets;
  static constexpr rank_t max_buckets = 256;

  // https://stackoverflow.com/questions/2422712/rounding-integer-division-instead-of-truncating
  static uint64_t round_div(const uint64_t dividend, const uint64_t divisor)
  {
    return (dividend + (divisor / 2)) / divisor;
  }

  static uint32_t round_div(const uint32_t dividend, const uint32_t divisor)
  {
    return (dividend + (divisor / 2)) / divisor;
  }

  bool invariant() const
  {
    observation_t sum{};
    for (const auto& bucket : _buckets) {
      sum += bucket.get_count();
    }
    assert(sum == _observations);
    assert(_buckets.size() <= max_buckets);
    return (sum == _observations && _buckets.size() <= max_buckets);
  }

  observation_t get_min() const
  {
    compact_buckets();
    return _buckets.size() ? _buckets.front().get_rank(1) : 0;
  }

  observation_t get_max() const
  {
    compact_buckets();
    return _buckets.size() ? _buckets.back().get_rank(_buckets.back().get_count()) : 0;
  }

  /// ensure we have room to add another bucket
  // makes sure the buckets are sorted correctly and non-overlapping
  void compact_buckets() const
  {
    if (_compacted)   return;
    assert(invariant());
    std::ranges::sort(_buckets,
                      [](const Bucket& l, const Bucket& r) { return (l.get_min() < r.get_min()); });

    const rank_t target_bucket_size = 1+round_div(_observations, max_buckets);
    Bucket* p_last_bucket = nullptr;

    for (auto bucket_it = std::begin(_buckets); bucket_it != std::end(_buckets); ) {
      if (p_last_bucket &&
          (p_last_bucket->get_count() < target_bucket_size ||
           bucket_it->is_overlapping(*p_last_bucket))) {
        *p_last_bucket += *bucket_it;   // merge
        bucket_it = _buckets.erase(bucket_it);
      } else {
        p_last_bucket = &(*bucket_it);   // assuming iterator may not be a simple pointer
        ++bucket_it;
      }
    }

    _compacted = true;
    assert(invariant());
    assert(_buckets.size() < max_buckets);
  }

  // returns the number of values <= threshold
  rank_t count_low_values(const observation_t threshold) const
  {
    compact_buckets();
    rank_t low_count = 0;
    for (const auto& bucket : _buckets) {
      if (bucket.get_max() <= threshold) {
        low_count += bucket.get_count();   // full bucket
      } else {                             // partial bucket
        for (rank_t rank = 1; rank <= bucket.get_count(); ++rank) {
          if (bucket.get_rank(rank) <= threshold) {
            ++low_count;
          } else {
            break;
          }
        }
        break;
      }
    }
    return low_count;
  }

  // returns the number of values >= threshold
  rank_t count_high_values(const observation_t threshold) const
  {
    compact_buckets();
    rank_t high_count = 0;
    for (auto bucket_it = std::crbegin(_buckets); bucket_it != std::crend(_buckets); ++bucket_it) {
      if (bucket_it->get_min() >= threshold) {
        high_count += bucket_it->get_count();   // full bucket
      } else {                                  // partial bucket
        for (rank_t rank = bucket_it->get_count(); rank >= 1; --rank) {
          if (bucket_it->get_rank(rank) >= threshold) {
            ++high_count;
          } else {
            break;
          }
        }
        break;
      }
    }
    return high_count;
  }

  // this is surprisingly accurate, but is only used internally
  // for <100 observations, using the IQR (25/75 quantiles) is appropriate
  // for >100 observations, using 20/80 quantiles might be more accurate
  // we always use the IQR to yield more consistent results
  // https://en.wikipedia.org/wiki/Robust_measures_of_scale
  // https://en.wikipedia.org/wiki/Scale_parameter
  // https://en.wikipedia.org/wiki/Error_function
  double get_standard_deviation() const
  {
    compact_buckets();
    static constexpr double iqr_unbiased = 1.35623115191269; // 2*sqrt(2)*erfc(0.5)
    return get_interquantile_range() / iqr_unbiased;
  }
};

std::ostream&
operator<<(std::ostream& os, const Histogram& histogram) {
  return histogram.write(os);
}

} // namespace histogram

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
class LinuxEvent {
 public:
  LinuxEvent() : _num_events(0) {}

  LinuxEvent(const std::string_view name, const uint32_t event_type, const uint64_t event) {
    _num_events = 1;
    _name1 = name;
    _fd1 = open_event(name, event_type, event, -1);
  }

  LinuxEvent(const std::string_view name1, const uint32_t event_type1, const uint64_t event1,
             const std::string_view name2, const uint32_t event_type2, const uint64_t event2) {
    _num_events = 2;
    _name1 = name1;
    _fd1 = open_event(name1, event_type1, event1, -1);
    _name2 = name2;
    _fd2 = open_event(name2, event_type2, event2, _fd1);

    reset_event(name1, _fd1, (_num_events == 1 ? Group::single : Group::leader));
  }

  LinuxEvent(const std::string_view name1, const uint32_t event_type1, const uint64_t event1,
             const std::string_view name2, const uint32_t event_type2, const uint64_t event2,
             const std::string_view name3, const uint32_t event_type3, const uint64_t event3) {
    _num_events = 3;
    _name1 = name1;
    _fd1 = open_event(name1, event_type1, event1, -1);
    _name2 = name2;
    _fd2 = open_event(name2, event_type2, event2, _fd1);
    _name3 = name3;
    _fd3 = open_event(name3, event_type3, event3, _fd1);

    reset_event(name1, _fd1, (_num_events == 1 ? Group::single : Group::leader));
  }

  LinuxEvent(const std::string_view name1, const uint32_t event_type1, const uint64_t event1,
             const std::string_view name2, const uint32_t event_type2, const uint64_t event2,
             const std::string_view name3, const uint32_t event_type3, const uint64_t event3,
             const std::string_view name4, const uint32_t event_type4, const uint64_t event4) {
    _num_events = 4;
    _name1 = name1;
    _fd1 = open_event(name1, event_type1, event1, -1);
    _name2 = name2;
    _fd2 = open_event(name2, event_type2, event2, _fd1);
    _name3 = name3;
    _fd3 = open_event(name3, event_type3, event3, _fd1);
    _name4 = name4;
    _fd4 = open_event(name4, event_type4, event4, _fd1);

    reset_event(name1, _fd1, (_num_events == 1 ? Group::single : Group::leader));
  }

  ~LinuxEvent() {
    disable_event(_name1, _fd1, (_num_events == 1 ? Group::single : Group::leader));

    switch (_num_events) {
      case 4: close_event(_name4, _fd4);   /* fall-through */
      case 3: close_event(_name3, _fd3);   /* fall-through */
      case 2: close_event(_name2, _fd2);   /* fall-through */
      case 1: close_event(_name1, _fd1);
    }
  }

  void reset_events() {
    reset_event(_name1, _fd1, (_num_events == 1 ? Group::single : Group::leader));
  }

  void enable_events() {
    enable_event(_name1, _fd1, (_num_events == 1 ? Group::single : Group::leader));
  }

  uint64_t read_event1() {
    assert(_num_events >= 1);
    return read_event(_name1, _fd1);
  }

  uint64_t read_event2() {
    assert(_num_events >= 2);
    return read_event(_name2, _fd2);
  }

  uint64_t read_event3() {
    assert(_num_events >= 3);
    return read_event(_name3, _fd3);
  }

  uint64_t read_event4() {
    assert(_num_events >= 4);
    return read_event(_name4, _fd4);
  }

 private:
  enum class Group { leader, single };
  int _num_events;
  std::string_view _name1, _name2, _name3, _name4;
  int _fd1, _fd2, _fd3, _fd4;

  static int perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu,
                             int group_fd, unsigned long flags) {
    return static_cast<int>(syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags));
  }

  static int open_event(const std::string_view name,
                        const uint32_t event_type, const uint64_t event, const int group_fd) {
    struct perf_event_attr perf_event_attr{
      .size = sizeof(perf_event_attr);
      .type = event_type;
      .config = event;
      .disabled = 1;
      .exclude_kernel = 1;
      .exclude_hv = 1;
    };

    const int fd = perf_event_open(&perf_event_attr, 0, -1, group_fd, 0);
    if (fd == -1) {
      std::cerr << "ERROR: LinuxEvent::open_event: " << name << ": " << std::strerror(errno) << std::endl;
      std::exit(EXIT_FAILURE);
    }

    return fd;
  }

  static void reset_event(const std::string_view name, const int fd, const Group group) {
    const int status = ioctl(fd, PERF_EVENT_IOC_RESET, (group == Group::leader ? PERF_IOC_FLAG_GROUP : 0));
    if (status == -1) {
      std::cerr << "ERROR: LinuxEvent::reset_event: " << name << ": " << std::strerror(errno) << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  static void disable_event(const std::string_view name, const int fd, const Group group) {
    const int status = ioctl(fd, PERF_EVENT_IOC_DISABLE, (group == Group::leader ? PERF_IOC_FLAG_GROUP : 0));
    if (status == -1) {
      std::cerr << "ERROR: LinuxEvent::disable_event: " << name << ": " << std::strerror(errno) << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  static void enable_event(const std::string_view name, const int fd, const Group group) {
    const int status = ioctl(fd, PERF_EVENT_IOC_ENABLE, (group == Group::leader ? PERF_IOC_FLAG_GROUP : 0));
    if (status == -1) {
      std::cerr << "ERROR: LinuxEvent::enable_event: " << name << ": " << std::strerror(errno) << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  static void close_event(const std::string_view name, const int fd) {
    const int status = close(fd);
    if (status == -1) {
      std::cerr << "ERROR: LinuxEvent::close_event: " << name << ": " << std::strerror(errno) << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  static uint64_t read_event(const std::string_view name, const int fd) {
    uint64_t count;
    const ssize_t bytes_read = read(fd, &count, sizeof(count));
    if (bytes_read == -1) {
      std::cerr << "ERROR: LinuxEvent::read_event: " << name << ": " << std::strerror(errno) << std::endl;
      std::exit(EXIT_FAILURE);
    }
    return count;
  }
};

// ---------------------------------------------------------------------------
class LinuxEventsData {
 public:
  explicit LinuxEventsData() = default;

  static void write_header(std::ostream &os) {
    os << "TotalCpuSec,TotalTaskIdlePct,TotalPageFaultMajorPerSec,"
       << "TotalCyclesPerInstr,TotalIssueStallPct,TotalRetireStallPct,TotalCacheMissPct,TotalBranchMissPct,"
       << "SelfCpuSec,SelfTaskIdlePct,SelfPageFaultMajorPerSec,"
       << "SelfCyclesPerInstr,SelfIssueStallPct,SelfRetireStallPct,SelfCacheMissPct,SelfBranchMissPct";
  }

  void write_data(std::ostream &os) {
    os << get_cpu_seconds() << ','
       << get_task_idle_pct() << ','
       << get_page_fault_major_per_sec() << ','
       << get_cycles_per_instr() << ','
       << get_issue_stall_pct() << ','
       << get_retire_stall_pct() << ','
       << get_cache_miss_pct() << ','
       << get_branch_miss_pct();
  }

  LinuxEventsData &operator+=(const LinuxEventsData &rhs) {
    _fd_sw_cpu_clock += rhs._fd_sw_cpu_clock;
    _fd_sw_task_clock += rhs._fd_sw_task_clock;
    _fd_sw_page_faults += rhs._fd_sw_page_faults;
    _fd_sw_context_switches += rhs._fd_sw_context_switches;
    _fd_sw_cpu_migrations += rhs._fd_sw_cpu_migrations;
    _fd_sw_page_faults_min += rhs._fd_sw_page_faults_min;
    _fd_sw_page_faults_maj += rhs._fd_sw_page_faults_maj;
    _fd_sw_alignment_faults += rhs._fd_sw_alignment_faults;
    _fd_sw_emulation_faults += rhs._fd_sw_emulation_faults;
    _fd_hw_cpu_cycles += rhs._fd_hw_cpu_cycles;
    _fd_hw_instructions += rhs._fd_hw_instructions;
    _fd_hw_stalled_cycles_frontend += rhs._fd_hw_stalled_cycles_frontend;
    _fd_hw_stalled_cycles_backend += rhs._fd_hw_stalled_cycles_backend;
    _fd_hw_cache_references += rhs._fd_hw_cache_references;
    _fd_hw_cache_misses += rhs._fd_hw_cache_misses;
    _fd_hw_branch_instructions += rhs._fd_hw_branch_instructions;
    _fd_hw_branch_misses += rhs._fd_hw_branch_misses;
    return *this;
  }

  friend LinuxEventsData operator+(LinuxEventsData lhs, const LinuxEventsData &rhs) {
    lhs += rhs;
    return lhs;
  }

  LinuxEventsData &operator-=(const LinuxEventsData &rhs) {
    _fd_sw_cpu_clock -= rhs._fd_sw_cpu_clock;
    _fd_sw_task_clock -= rhs._fd_sw_task_clock;
    _fd_sw_page_faults -= rhs._fd_sw_page_faults;
    _fd_sw_context_switches -= rhs._fd_sw_context_switches;
    _fd_sw_cpu_migrations -= rhs._fd_sw_cpu_migrations;
    _fd_sw_page_faults_min -= rhs._fd_sw_page_faults_min;
    _fd_sw_page_faults_maj -= rhs._fd_sw_page_faults_maj;
    _fd_sw_alignment_faults -= rhs._fd_sw_alignment_faults;
    _fd_sw_emulation_faults -= rhs._fd_sw_emulation_faults;
    _fd_hw_cpu_cycles -= rhs._fd_hw_cpu_cycles;
    _fd_hw_instructions -= rhs._fd_hw_instructions;
    _fd_hw_stalled_cycles_frontend -= rhs._fd_hw_stalled_cycles_frontend;
    _fd_hw_stalled_cycles_backend -= rhs._fd_hw_stalled_cycles_backend;
    _fd_hw_cache_references -= rhs._fd_hw_cache_references;
    _fd_hw_cache_misses -= rhs._fd_hw_cache_misses;
    _fd_hw_branch_instructions -= rhs._fd_hw_branch_instructions;
    _fd_hw_branch_misses -= rhs._fd_hw_branch_misses;
    return *this;
  }

  friend LinuxEventsData operator-(LinuxEventsData lhs, const LinuxEventsData &rhs) {
    lhs -= rhs;
    return lhs;
  }

  double get_cpu_seconds() {
    return static_cast<double>(_fd_sw_cpu_clock) / 1'000'000'000.0;
  }

  double get_task_idle_pct() {
    return 1.0 - (static_cast<double>(_fd_sw_task_clock) / static_cast<double>(_fd_sw_cpu_clock));
  }

  double get_page_fault_major_per_sec() {
    return static_cast<double>(_fd_sw_page_faults_maj) / get_cpu_seconds();
  }

  double get_cycles_per_instr() {
    return static_cast<double>(_fd_hw_cpu_cycles) / static_cast<double>(_fd_hw_instructions);
  }

  double get_issue_stall_pct() {
    return static_cast<double>(_fd_hw_stalled_cycles_frontend) / static_cast<double>(_fd_hw_cpu_cycles);
  }

  double get_retire_stall_pct() {
    return static_cast<double>(_fd_hw_stalled_cycles_backend) / static_cast<double>(_fd_hw_cpu_cycles);
  }

  double get_cache_miss_pct() {
    return static_cast<double>(_fd_hw_cache_misses) / static_cast<double>(_fd_hw_cache_references);
  }

  double get_branch_miss_pct() {
    return static_cast<double>(_fd_hw_branch_misses) / static_cast<double>(_fd_hw_branch_instructions);
  }

// ---------------------------------------------------------------------------
  uint64_t _fd_sw_cpu_clock;         // This reports the CPU clock, a high-resolution per-CPU timer. (nanos)
  uint64_t _fd_sw_task_clock;        // This reports a clock count specific to the task that is running. (nanos)
  uint64_t _fd_sw_page_faults;       // This reports the number of page faults.
  uint64_t _fd_sw_context_switches;  // This counts context switches.
  uint64_t _fd_sw_cpu_migrations;    // This reports the number of times the process has migrated to a new CPU.
  uint64_t _fd_sw_page_faults_min;   // This counts the number of minor page faults.
  uint64_t _fd_sw_page_faults_maj;   // This counts the number of major page faults. These required disk I/O to handle.
  uint64_t _fd_sw_alignment_faults;  // This counts the number of alignment faults. Zero on x86.
  uint64_t _fd_sw_emulation_faults;  // This counts the number of emulation faults.

  uint64_t _fd_hw_cpu_cycles;               // Total cycles.
  uint64_t _fd_hw_instructions;             // Retired instructions.
  uint64_t _fd_hw_stalled_cycles_frontend;  // Stalled cycles during issue.
  uint64_t _fd_hw_stalled_cycles_backend;   // Stalled cycles during retirement.

  uint64_t _fd_hw_cache_references;  // Cache accesses.  Usually this indicates Last Level Cache accesses.
  uint64_t _fd_hw_cache_misses;      // Cache misses.  Usually this indicates Last Level Cache misses.

  uint64_t _fd_hw_branch_instructions; // Retired branch instructions.
  uint64_t _fd_hw_branch_misses;       // Mispredicted branch instructions.
};

// ---------------------------------------------------------------------------
class LinuxEvents {
 public:
  explicit LinuxEvents() {
    open_events();
  }

  ~LinuxEvents() = default;

  void enable_events() {
    _fd_sw_cpu_clock.enable_events();
    _fd_sw_task_clock.enable_events();
    _fd_sw_page_faults.enable_events();
    _fd_sw_context_switches.enable_events();
    _fd_sw_cpu_migrations.enable_events();
    _fd_sw_page_faults_min.enable_events();
    _fd_sw_page_faults_maj.enable_events();
    _fd_sw_alignment_faults.enable_events();
    _fd_sw_emulation_faults.enable_events();

    _fd_hw_cpu_cycles_instr_group.enable_events();
    _fd_hw_cache_references_misses_group.enable_events();
    _fd_hw_branch_instructions_misses_group.enable_events();
  }

  LinuxEventsData get_snapshot() {
    LinuxEventsData data{};
    data._fd_sw_cpu_clock = _fd_sw_cpu_clock.read_event1();
    data._fd_sw_task_clock = _fd_sw_task_clock.read_event1();
    data._fd_sw_page_faults = _fd_sw_page_faults.read_event1();
    data._fd_sw_context_switches = _fd_sw_context_switches.read_event1();
    data._fd_sw_cpu_migrations = _fd_sw_cpu_migrations.read_event1();
    data._fd_sw_page_faults_min = _fd_sw_page_faults_min.read_event1();
    data._fd_sw_page_faults_maj = _fd_sw_page_faults_maj.read_event1();
    data._fd_sw_alignment_faults = _fd_sw_alignment_faults.read_event1();
    data._fd_sw_emulation_faults = _fd_sw_emulation_faults.read_event1();

    data._fd_hw_cpu_cycles = _fd_hw_cpu_cycles_instr_group.read_event1();
    data._fd_hw_instructions = _fd_hw_cpu_cycles_instr_group.read_event2();
    data._fd_hw_stalled_cycles_frontend = _fd_hw_cpu_cycles_instr_group.read_event3();
    data._fd_hw_stalled_cycles_backend = _fd_hw_cpu_cycles_instr_group.read_event4();

    data._fd_hw_cache_references = _fd_hw_cache_references_misses_group.read_event1();
    data._fd_hw_cache_misses = _fd_hw_cache_references_misses_group.read_event2();
    data._fd_hw_branch_instructions = _fd_hw_branch_instructions_misses_group.read_event1();
    data._fd_hw_branch_misses = _fd_hw_branch_instructions_misses_group.read_event2();
    return data;
  }

 private:
  LinuxEvent _fd_sw_cpu_clock;         // This reports the CPU clock, a high-resolution per-CPU timer.
  LinuxEvent _fd_sw_task_clock;        // This reports a clock count specific to the task that is running.
  LinuxEvent _fd_sw_page_faults;       // This reports the number of page faults.
  LinuxEvent _fd_sw_context_switches;  // This counts context switches.
  LinuxEvent _fd_sw_cpu_migrations;    // This reports the number of times the process has migrated to a new CPU.
  LinuxEvent _fd_sw_page_faults_min;   // This counts the number of minor page faults.
  LinuxEvent
      _fd_sw_page_faults_maj;   // This counts the number of major page faults. These required disk I/O to handle.
  LinuxEvent _fd_sw_alignment_faults;  // This counts the number of alignment faults.
  LinuxEvent _fd_sw_emulation_faults;  // This  counts the number of emulation faults.

  // Total cycles.
  // Retired instructions.
  // Stalled cycles during issue.
  // Stalled cycles during retirement.
  LinuxEvent _fd_hw_cpu_cycles_instr_group;

  // Cache accesses.  Usually this indicates Last Level Cache accesses.
  // Cache misses.  Usually this indicates Last Level Cache misses.
  LinuxEvent _fd_hw_cache_references_misses_group;

  // Retired branch instructions.
  // Mispredicted branch instructions.
  LinuxEvent _fd_hw_branch_instructions_misses_group;

  void open_events() {
    _fd_sw_cpu_clock = LinuxEvent("PERF_COUNT_SW_CPU_CLOCK",
                                  PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_CLOCK);
    _fd_sw_task_clock = LinuxEvent("PERF_COUNT_SW_TASK_CLOCK",
                                   PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK);
    _fd_sw_page_faults = LinuxEvent("PERF_COUNT_SW_PAGE_FAULTS",
                                    PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS);
    _fd_sw_context_switches = LinuxEvent("PERF_COUNT_SW_CONTEXT_SWITCHES",
                                         PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES);
    _fd_sw_cpu_migrations = LinuxEvent("PERF_COUNT_SW_CPU_MIGRATIONS",
                                       PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS);
    _fd_sw_page_faults_min = LinuxEvent("PERF_COUNT_SW_PAGE_FAULTS_MIN",
                                        PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MIN);
    _fd_sw_page_faults_maj = LinuxEvent("PERF_COUNT_SW_PAGE_FAULTS_MAJ",
                                        PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MAJ);
    _fd_sw_alignment_faults = LinuxEvent("PERF_COUNT_SW_ALIGNMENT_FAULTS",
                                         PERF_TYPE_SOFTWARE, PERF_COUNT_SW_ALIGNMENT_FAULTS);
    _fd_sw_emulation_faults = LinuxEvent("PERF_COUNT_SW_EMULATION_FAULTS",
                                         PERF_TYPE_SOFTWARE, PERF_COUNT_SW_EMULATION_FAULTS);

    _fd_hw_cpu_cycles_instr_group =
        LinuxEvent("PERF_COUNT_HW_CPU_CYCLES",
                   PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES,
                   "PERF_COUNT_HW_INSTRUCTIONS",
                   PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS,
                   "PERF_COUNT_HW_STALLED_CYCLES_FRONTEND",
                   PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND,
                   "PERF_COUNT_HW_STALLED_CYCLES_BACKEND",
                   PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND);

    _fd_hw_cache_references_misses_group =
        LinuxEvent("PERF_COUNT_HW_CACHE_REFERENCES",
                   PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES,
                   "PERF_COUNT_HW_CACHE_MISSES",
                   PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
    _fd_hw_branch_instructions_misses_group =
        LinuxEvent("PERF_COUNT_HW_BRANCH_INSTRUCTIONS",
                   PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS,
                   "PERF_COUNT_HW_BRANCH_MISSES",
                   PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
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

// ---------------------------------------------------------------------------
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
