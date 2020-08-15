#pragma once

#include <memory>

#include "Lucid/Core/Ref.h"

void InitializeCore();
void ShutdownCore();

using byte = uint8_t;
using RendererID = uint32_t;

// Expansion of __VA_ARGS__ to get past MSVC "bug"
#define LD_EXPAND_VARGS(x) x

#define BIT(x) (1 << x)

// Definition for binding callback events (for event listener)
#define LD_BIND_EVENT_FN(fn) std::bind(&##fn, this, std::placeholders::_1)

#include "Lucid/Core/Assert.h"

// Pointer wrappers
template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}