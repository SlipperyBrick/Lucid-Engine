#include "ldpch.h"

#define LUCID_BUILD_ID "v0.1a"

void InitializeCore()
{
	Log::Init();

	LD_CORE_TRACE("Lucid Engine {}", LUCID_BUILD_ID);
	LD_CORE_TRACE("Initializing...");
}

void ShutdownCore()
{
	LD_CORE_TRACE("Shutting down...");
}