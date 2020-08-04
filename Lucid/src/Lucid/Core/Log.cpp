#include "ldpch.h"

#include "Log.h"

std::shared_ptr<spdlog::logger> Log::s_CoreLogger;

// Sets the print format of messages, creates a new console logger and sets verbosity of messages
void Log::Init()
{
	// Set the format for how messages will be logged (uses spdlog pattern flags)
	spdlog::set_pattern("%^[%T] %n: %v%$");

	// Create a colour multi-threaded logger
	s_CoreLogger = spdlog::stdout_color_mt("LUCID");

	// Set verbosity of messages to log
	s_CoreLogger->set_level(spdlog::level::trace);
}