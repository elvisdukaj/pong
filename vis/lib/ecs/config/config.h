#ifndef ENTT_CONFIG_CONFIG_H
#define ENTT_CONFIG_CONFIG_H

#include "version.h"

#if defined(ENTT_IMPORT_STD)
import std;
#endif

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#if defined(__cpp_exceptions) && !defined(ENTT_NOEXCEPTION)
#define ENTT_CONSTEXPR
#define ENTT_THROW throw
#define ENTT_TRY try
#define ENTT_CATCH catch (...)
#else
#define ENTT_CONSTEXPR constexpr // use only with throwing functions (waiting for C++20)
#define ENTT_THROW
#define ENTT_TRY if (true)
#define ENTT_CATCH if (false)
#endif

#if not defined(ENTT_IMPORT_STD) && __has_include(<version>)
#include <version>
#endif

#if defined(__cpp_consteval)
#define ENTT_CONSTEVAL consteval
#endif

#ifndef ENTT_CONSTEVAL
#define ENTT_CONSTEVAL constexpr
#endif

#ifdef ENTT_USE_ATOMIC
#if not defined(ENTT_IMPORT_STD)
#include <atomic>
#endif

#define ENTT_MAYBE_ATOMIC(Type) std::atomic<Type>
#else
#define ENTT_MAYBE_ATOMIC(Type) Type
#endif

#ifndef ENTT_ID_TYPE
#if defined(ENTT_IMPORT_STD)
import std;
#else
#include <cstdint> // provides coverage for types in the std namespace
#endif
#define ENTT_ID_TYPE std::uint32_t
#endif

#ifndef ENTT_SPARSE_PAGE
#define ENTT_SPARSE_PAGE 4096
#endif

#ifndef ENTT_PACKED_PAGE
#define ENTT_PACKED_PAGE 1024
#endif

#ifdef ENTT_DISABLE_ASSERT
#undef ENTT_ASSERT
#define ENTT_ASSERT(condition, msg) (void(0))
#elif !defined ENTT_ASSERT
#include <cassert>
#define ENTT_ASSERT(condition, msg) assert(((condition) && (msg)))
#endif

#ifdef ENTT_DISABLE_ASSERT
#undef ENTT_ASSERT_CONSTEXPR
#define ENTT_ASSERT_CONSTEXPR(condition, msg) (void(0))
#elif !defined ENTT_ASSERT_CONSTEXPR
#define ENTT_ASSERT_CONSTEXPR(condition, msg) ENTT_ASSERT(condition, msg)
#endif

#define ENTT_FAIL(msg) ENTT_ASSERT(false, msg);

#ifdef ENTT_NO_ETO
#define ENTT_ETO_TYPE(Type) void
#else
#define ENTT_ETO_TYPE(Type) Type
#endif

#ifdef ENTT_NO_MIXIN
#define ENTT_STORAGE(Mixin, ...) __VA_ARGS__
#else
#define ENTT_STORAGE(Mixin, ...) Mixin<__VA_ARGS__>
#endif

#ifdef ENTT_STANDARD_CPP
#define ENTT_NONSTD false
#else
#define ENTT_NONSTD true
#if defined __clang__ || defined __GNUC__
#define ENTT_PRETTY_FUNCTION __PRETTY_FUNCTION__
#define ENTT_PRETTY_FUNCTION_PREFIX '='
#define ENTT_PRETTY_FUNCTION_SUFFIX ']'
#elif defined _MSC_VER
#define ENTT_PRETTY_FUNCTION __FUNCSIG__
#define ENTT_PRETTY_FUNCTION_PREFIX '<'
#define ENTT_PRETTY_FUNCTION_SUFFIX '>'
#endif
#endif

#if defined _MSC_VER
#pragma detect_mismatch("entt.version", ENTT_VERSION)
#pragma detect_mismatch("entt.noexcept", ENTT_XSTR(ENTT_TRY))
#pragma detect_mismatch("entt.id", ENTT_XSTR(ENTT_ID_TYPE))
#pragma detect_mismatch("entt.nonstd", ENTT_XSTR(ENTT_NONSTD))
#endif

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif
