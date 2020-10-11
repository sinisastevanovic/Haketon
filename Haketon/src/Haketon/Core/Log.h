#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Haketon
{
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define HK_CORE_TRACE(...)	::Haketon::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define HK_CORE_INFO(...)	::Haketon::Log::GetCoreLogger()->info(__VA_ARGS__)
#define HK_CORE_WARN(...)	::Haketon::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define HK_CORE_ERROR(...)	::Haketon::Log::GetCoreLogger()->error(__VA_ARGS__)
#define HK_CORE_CRIT(...)	::Haketon::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define HK_TRACE(...)		::Haketon::Log::GetClientLogger()->trace(__VA_ARGS__)
#define HK_INFO(...)		::Haketon::Log::GetClientLogger()->info(__VA_ARGS__)
#define HK_WARN(...)		::Haketon::Log::GetClientLogger()->warn(__VA_ARGS__)
#define HK_ERROR(...)		::Haketon::Log::GetClientLogger()->error(__VA_ARGS__)
#define HK_CRIT(...)		::Haketon::Log::GetClientLogger()->critical(__VA_ARGS__)

// TODO: Support custom LogCategories like UE4