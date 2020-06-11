#pragma once

// __VA_ARGS__ expansion to get past MSVC "bug"
#define LD_EXPAND_VARGS(x) x

#include "Lucid/Core/Assert.h"

#define BIT(x) (1 << x)

#define LD_BIND_EVENT_FN(fn) std::bind(&##fn, this, std::placeholders::_1)