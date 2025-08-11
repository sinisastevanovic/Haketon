#pragma once

#include "Haketon/Core/PlatformDetection.h"

#ifdef HK_PLATFORM_WINDOWS
    #define HK_DEBUG_BREAK __debugbreak()
#else
    #define HK_DEBUG_BREAK
#endif

#ifdef HK_DEBUG
    #define HK_ENABLE_ASSERTS
#endif

#define HK_ENABLE_VERIFY

#define HK_GET_ASSERT_MACRO(arg1, arg2, macro, ...) macro
#define HK_EXPAND_MACRO(macro) macro

#define HK_ASSERT_INTERNAL_WITH_MSG(type, condition, ...)   \
    {                                                       \
        if (!(condition)) {                                 \
            ::Haketon::Log::PrintAssertMessage(type, "Assertion Failed: ", __VA_ARGS__); \
            HK_DEBUG_BREAK;                                 \
        }                                                   \
    }

#define HK_ASSERT_INTERNAL_NO_MSG(type, condition)  \
    {                                               \
        if (!(condition)) {                         \
            ::Haketon::Log::PrintAssertMessage(type, "Assertion Failed: ", #condition); \
            HK_DEBUG_BREAK;                         \
        }                                           \
    }

#define HK_VERIFY_INTERNAL_WITH_MSG(type, condition, ...)   \
    {                                                       \
        if (!(condition)) {                                 \
            ::Haketon::Log::PrintAssertMessage(type, "Verify Failed: ", __VA_ARGS__); \
            HK_DEBUG_BREAK;                                 \
        }                                                   \
    }

#define HK_VERIFY_INTERNAL_NO_MSG(type, condition)  \
    {                                               \
        if (!(condition)) {                         \
            ::Haketon::Log::PrintAssertMessage(type, "Verify Failed: ", #condition); \
            HK_DEBUG_BREAK;                         \
        }                                           \
    }                                      

#ifdef HK_ENABLE_ASSERTS
    #define HK_CORE_ASSERT(...) HK_EXPAND_MACRO(HK_GET_ASSERT_MACRO(__VA_ARGS__,    \
        HK_ASSERT_INTERNAL_WITH_MSG,                                                \
        HK_ASSERT_INTERNAL_NO_MSG)(::Haketon::Log::Type::Core, __VA_ARGS__))

    #define HK_ASSERT(...) HK_EXPAND_MACRO(HK_GET_ASSERT_MACRO(__VA_ARGS__,         \
        HK_ASSERT_INTERNAL_WITH_MSG,                                                \
        HK_ASSERT_INTERNAL_NO_MSG)(::Haketon::Log::Type::Client, __VA_ARGS__))

#else
    #define HK_CORE_ASSERT(condition, ...)
    #define HK_ASSERT(condition, ...)
#endif

#ifdef HK_ENABLE_VERIFY
    #define HK_CORE_VERIFY(...) HK_EXPAND_MACRO(HK_GET_ASSERT_MACRO(__VA_ARGS__,    \
        HK_VERIFY_INTERNAL_WITH_MSG,                                                \
        HK_VERIFY_INTERNAL_NO_MSG)(::Haketon::Log::Type::Core, __VA_ARGS__))

    #define HK_VERIFY(...) HK_EXPAND_MACRO(HK_GET_ASSERT_MACRO(__VA_ARGS__,         \
        HK_VERIFY_INTERNAL_WITH_MSG,                                                \
        HK_VERIFY_INTERNAL_NO_MSG)(::Haketon::Log::Type::Client, __VA_ARGS__))
#else
    #define HK_CORE_VERIFY(condition, ...)
    #define HK_VERIFY(condition, ...)
#endif