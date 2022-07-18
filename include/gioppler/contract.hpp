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
#ifndef GIOPPLER_CONTRACT_HPP
#define GIOPPLER_CONTRACT_HPP

#if __cplusplus < 202002L
#error C++20 or newer support required to use this library.
#endif

#include <exception>
#include <functional>
#include <memory>
#include <iostream>
#include <source_location>
#include <stdexcept>

#include "gioppler/utility.hpp"
#include "gioppler/sink.hpp"

// -----------------------------------------------------------------------------
/// Contracts to ensure correct program behavior.
// These print messages to the log and throw exceptions, as needed.
// These functions will always succeed.
namespace gioppler::contract
{

// -----------------------------------------------------------------------------
/// a contract condition has been violated
//
// logic_error are errors that are a consequence of faulty logic
// within the program such as violating logical preconditions or
// class invariants and may be preventable.
class contract_violation : public std::logic_error
{
public:
    explicit contract_violation(std::string what_arg)
    : std::logic_error(what_arg) { }

    virtual ~contract_violation() noexcept { }

    contract_violation(const contract_violation&) = default;
    contract_violation(contract_violation&&) = default;
    contract_violation& operator=(const contract_violation&) = default;
    contract_violation& operator=(contract_violation&&) = default;
};

// -----------------------------------------------------------------------------
/// errors that arise because an argument value has not been accepted
// the function's expectation of its arguments upon entry into the function
// prints error to std::cerr and throws exception
void argument(const bool condition,
              [[maybe_unused]] const std::source_location &source_location =
                std::source_location::current())
{
  if (!condition) [[unlikely]] {
    const std::string message =
      format("ERROR: {}: invalid argument\n",
             format_source_location(source_location));
    std::shared_ptr<Record> record{make_shared_init_list<Record>({
      {"category", "contract"},
      {"subcategory", "argument"},
      {"message", message}
    })};
    record->merge(source_location_to_record(source_location));
    sink::g_sink_manager.write_record(record);
    throw contract_violation{message};
  }
}

// -----------------------------------------------------------------------------
/// expect conditions are like preconditions
// the function's expectation of the state of other objects upon entry into the function
// prints error to std::cerr and throws exception
void expect(const bool condition,
            [[maybe_unused]] const std::source_location &source_location =
              std::source_location::current())
{
  if (!condition) [[unlikely]] {
    const std::string message =
      format("ERROR: {}: expect condition failed\n",
             format_source_location(source_location));
    std::shared_ptr<Record> record{make_shared_init_list<Record>({
      {"category", "contract"},
      {"subcategory", "expect"},
      {"message", message}
    })};
    record->merge(source_location_to_record(source_location));
    sink::g_sink_manager.write_record(record);
    throw contract_violation{message};
  }
}

// -----------------------------------------------------------------------------
/// confirms a condition that should be satisfied where it appears in a function body
// prints error to std::cerr and throws exception
void confirm(const bool condition,
            [[maybe_unused]] const std::source_location &source_location =
              std::source_location::current())
{
  if (!condition) [[unlikely]] {
    const std::string message =
      format("ERROR: {}: confirm failed\n",
             format_source_location(source_location));
    std::shared_ptr<Record> record{make_shared_init_list<Record>({
      {"category", "contract"},
      {"subcategory", "confirm"},
      {"message", message}
    })};
    record->merge(source_location_to_record(source_location));
    sink::g_sink_manager.write_record(record);
    throw contract_violation{message};
  }
}

// -----------------------------------------------------------------------------
// turn off pesky warning about throwing from inside a catch clause
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wterminate"

// -----------------------------------------------------------------------------
/// invariant condition to check on scope entry and exit
class Invariant {
 public:
  Invariant() = delete;

  // check invariant on scope entry
  explicit Invariant(std::function<bool()> condition_function,
            [[maybe_unused]] const std::source_location &source_location =
              std::source_location::current())
  : _uncaught_exceptions(std::uncaught_exceptions()),
    _condition_function(std::move(condition_function)),
    _source_location(source_location)
  {
    if (!_condition_function()) [[unlikely]] {
      const std::string message =
        format("ERROR: {}: invariant failed on entry\n",
               format_source_location(_source_location));
        std::shared_ptr<Record> record{make_shared_init_list<Record>({
          {"category", "contract"},
          {"subcategory", "invariant"},
          {"message", message}
        })};
        record->merge(source_location_to_record(_source_location));
        sink::g_sink_manager.write_record(record);
        throw contract_violation{message};
    }
  }

  // recheck invariant on scope exit
  // https://en.cppreference.com/w/cpp/error/uncaught_exception
  // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4152.pdf
  ~Invariant() {
    if (!_condition_function()) [[unlikely]] {
      try {
        const std::string message =
          format("ERROR: {}: invariant failed on exit\n",
                 format_source_location(_source_location));
          std::shared_ptr<Record> record{make_shared_init_list<Record>({
            {"category", "contract"},
            {"subcategory", "invariant"},
            {"message", message}
          })};
          record->merge(source_location_to_record(_source_location));
          sink::g_sink_manager.write_record(record);
          throw contract_violation{message};
      } catch(...) {
        if (std::uncaught_exceptions() == _uncaught_exceptions) {
          throw;   // safe to rethrow
        } else {
          // unsafe to throw an exception - discard
        }
      }
    }
  }

 private:
  int _uncaught_exceptions;
  std::function<bool()> _condition_function;
  std::source_location _source_location;
};

// -----------------------------------------------------------------------------
/// ensure postcondition to check on scope exit
class Ensure {
 public:
  Ensure() = delete;

  explicit Ensure(std::function<bool()> condition_function,
         [[maybe_unused]] const std::source_location &source_location =
          std::source_location::current())
  : _uncaught_exceptions(std::uncaught_exceptions()),
    _condition_function(std::move(condition_function)),
    _source_location(source_location)
  { }

  // check condition only on scope exit
  // https://en.cppreference.com/w/cpp/error/uncaught_exception
  // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4152.pdf
  ~Ensure() {
    if (!_condition_function()) [[unlikely]] {
      try {
        const std::string message =
          format("ERROR: {}: ensure condition failed on exit\n",
                 format_source_location(_source_location));
          std::shared_ptr<Record> record{make_shared_init_list<Record>({
            {"category", "contract"},
            {"subcategory", "ensure"},
            {"message", message}
          })};
          record->merge(source_location_to_record(_source_location));
          sink::g_sink_manager.write_record(record);
          throw contract_violation{message};
      } catch(...) {
        if (std::uncaught_exceptions() == _uncaught_exceptions) {
          throw;   // safe to rethrow
        } else {
          // unsafe to throw an exception - discard
        }
      }
    }
  }

 private:
  int _uncaught_exceptions;
  std::function<bool()> _condition_function;
  std::source_location _source_location;
};

// -----------------------------------------------------------------------------
// restore diagnostic settings
#pragma GCC diagnostic pop

// -----------------------------------------------------------------------------
}   // namespace gioppler::contract

// -----------------------------------------------------------------------------
#endif // defined GIOPPLER_CONTRACT_HPP
