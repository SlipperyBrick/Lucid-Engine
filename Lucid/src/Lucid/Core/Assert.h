#pragma once

#include "Lucid/Core/Log.h"

// Assert definitions for debugging purposes
#define LD_ASSERT_NO_MESSAGE(condition) { if(!(condition)) { LD_CORE_ERROR("Assertion Failed"); __debugbreak(); } }
#define LD_ASSERT_MESSAGE(condition, ...) { if(!(condition)) { LD_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }

#define LD_ASSERT_RESOLVE(arg1, arg2, macro, ...) macro
#define LD_GET_ASSERT_MACRO(...) LD_EXPAND_VARGS(LD_ASSERT_RESOLVE(__VA_ARGS__, LD_ASSERT_MESSAGE, LD_ASSERT_NO_MESSAGE))

#define LD_ASSERT(...) LD_EXPAND_VARGS( LD_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#define LD_CORE_ASSERT(...) LD_EXPAND_VARGS( LD_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )