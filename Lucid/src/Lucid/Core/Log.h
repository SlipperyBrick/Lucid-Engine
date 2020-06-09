#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

class Log
{

public:

	static void Init();

	inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }

private:

	static std::shared_ptr<spdlog::logger> s_CoreLogger;
};

// Core logging macros
#define LD_CORE_TRACE(...)	Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LD_CORE_INFO(...)	Log::GetCoreLogger()->info(__VA_ARGS__)
#define LD_CORE_WARN(...)	Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LD_CORE_ERROR(...)	Log::GetCoreLogger()->error(__VA_ARGS__)
#define LD_CORE_FATAL(...)	Log::GetCoreLogger()->critical(__VA_ARGS__)