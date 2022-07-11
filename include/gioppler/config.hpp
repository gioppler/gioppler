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
#ifndef GIOPPLER_CONFIG_HPP
#define GIOPPLER_CONFIG_HPP

#if __cplusplus < 202002L
#error C++20 or newer support required to use this library.
#endif

#include <string>
#include <cstdint>

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
#endif // defined GIOPPLER_CONFIG_HPP
