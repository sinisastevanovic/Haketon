#include "hkpch.h"

#include "Log.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <vector>
#include <mutex>
#include <algorithm>

namespace Haketon
{
	std::shared_ptr<spdlog::logger> s_CoreLogger;
	std::shared_ptr<spdlog::logger> s_ClientLogger;
	std::vector<ILogHandler*> s_LogHandlers;
	std::mutex s_HandlersMutex;

	bool Log::s_Initialized = false;

	void Log::Init()
	{
		//spdlog::set_pattern("%^[%T] %n: %v%$");
		spdlog::set_pattern("[%d-%m-%Y %T] [%n] %^%l: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("Core Engine");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
		s_Initialized = true;
	}

	void Log::Shutdown()
	{
		s_CoreLogger.reset();
		s_ClientLogger.reset();
		spdlog::drop_all();
		s_Initialized = false;
	}

	bool Log::IsInitialized()
	{
		return s_Initialized;
	}

	void Log::RegisterLogHandler(ILogHandler* handler)
	{
		if (handler)
		{
			std::lock_guard<std::mutex> lock(s_HandlersMutex);
			s_LogHandlers.push_back(handler);
		}
	}

	void Log::UnregisterLogHandler(ILogHandler* handler)
	{
		if (handler)
		{
			std::lock_guard<std::mutex> lock(s_HandlersMutex);
			s_LogHandlers.erase(std::remove(s_LogHandlers.begin(), s_LogHandlers.end(), handler), s_LogHandlers.end());
		}
	}

	void Log::Impl_PrintMessage(Log::Type type, Log::Level level, std::string_view message)
	{
		auto logger = (type == Type::Core) ? s_CoreLogger : s_ClientLogger;
		switch (level)
		{
			case Level::Trace:
				logger->trace(message);
				break;
			case Level::Info:
				logger->info(message);
				break;
			case Level::Warn:
				logger->warn(message);
				break;
			case Level::Error:
				logger->error(message);
				break;
			case Level::Fatal:
				logger->critical(message);
				break;
		}

		// Notify handlers
		{
			std::lock_guard<std::mutex> lock(s_HandlersMutex);
			for (ILogHandler* handler : s_LogHandlers)
			{
				handler->HandleLog(type, level, message);
			}
		}
	}
}
