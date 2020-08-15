#include "ldpch.h"

// The engine saw over 20 iterations, 2 major milestones (the second milestone was integration of the editor)
#define LUCID_BUILD_ID "v2.1"

// Log to console successful init of logging system
void InitializeCore()
{
	Log::Init();

	LD_CORE_TRACE("Lucid Engine {}", LUCID_BUILD_ID);
	LD_CORE_TRACE("Initializing...");
}

// Log to console a successful shutdown
void ShutdownCore()
{
	LD_CORE_TRACE("Shutting down...");
}