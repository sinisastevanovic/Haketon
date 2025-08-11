#pragma once

#include "Core.h"
#include <string_view>
#include <fmt/format.h>

namespace spdlog
{
	class logger;
}

namespace Haketon
{
	class ILogHandler;
	
	class HK_API Log
	{
	public:
		enum class Type : uint8_t
		{
			Core = 0, Client = 1
		};
		enum class Level : uint8_t
		{
			Trace = 0, Info, Warn, Error, Fatal
		};
		
		static void Init();
		static void Shutdown();
		static bool IsInitialized();

		template<typename... Args>
		static void PrintMessage(Log::Type type, Log::Level level, fmt::format_string<Args...> fmt, Args&&... args);

		static void PrintMessage(Log::Type type, Log::Level level, std::string_view message);

		template<typename... Args>
		static void PrintAssertMessage(Log::Type type, std::string_view prefix, fmt::format_string<Args...> fmt, Args&&... args);

		static void PrintAssertMessage(Log::Type type, std::string_view prefix, std::string_view message);

		static void RegisterLogHandler(ILogHandler* handler);
		static void UnregisterLogHandler(ILogHandler* handler);

	public:
		static const char* LevelToString(Level level)
		{
			switch (level)
			{
				case Level::Trace: return "Trace";
				case Level::Info: return "Info";
				case Level::Warn: return "Warn";
				case Level::Error: return "Error";
				case Level::Fatal: return "Fatal";
			}
			return "";
		}
		static Level LevelFromString(std::string_view string)
		{
			if (string == "Trace") return Level::Trace;
			if (string == "Info") return Level::Info;
			if (string == "Warn") return Level::Warn;
			if (string == "Error") return Level::Error;
			if (string == "Fatal") return Level::Fatal;
			return Level::Trace;
		}

	private:
		static void Impl_PrintMessage(Log::Type type, Log::Level level, std::string_view message);

		static bool s_Initialized;
	};

	class HK_API ILogHandler
	{
	public:
		virtual ~ILogHandler() = default;
		virtual void HandleLog(Log::Type type, Log::Level level, std::string_view message) = 0;
	};
}

// Core log macros
#define HK_CORE_TRACE(...)	::Haketon::Log::PrintMessage(::Haketon::Log::Type::Core, ::Haketon::Log::Level::Trace, __VA_ARGS__)
#define HK_CORE_INFO(...)	::Haketon::Log::PrintMessage(::Haketon::Log::Type::Core, ::Haketon::Log::Level::Info, __VA_ARGS__)
#define HK_CORE_WARN(...)	::Haketon::Log::PrintMessage(::Haketon::Log::Type::Core, ::Haketon::Log::Level::Warn, __VA_ARGS__)
#define HK_CORE_ERROR(...)	::Haketon::Log::PrintMessage(::Haketon::Log::Type::Core, ::Haketon::Log::Level::Error, __VA_ARGS__)
#define HK_CORE_FATAL(...)	::Haketon::Log::PrintMessage(::Haketon::Log::Type::Core, ::Haketon::Log::Level::Fatal, __VA_ARGS__)

// Client log macros
#define HK_TRACE(...)		::Haketon::Log::PrintMessage(::Haketon::Log::Type::Client, ::Haketon::Log::Level::Trace, __VA_ARGS__)
#define HK_INFO(...)		::Haketon::Log::PrintMessage(::Haketon::Log::Type::Client, ::Haketon::Log::Level::Info, __VA_ARGS__)
#define HK_WARN(...)		::Haketon::Log::PrintMessage(::Haketon::Log::Type::Client, ::Haketon::Log::Level::Warn, __VA_ARGS__)
#define HK_ERROR(...)		::Haketon::Log::PrintMessage(::Haketon::Log::Type::Client, ::Haketon::Log::Level::Error, __VA_ARGS__)
#define HK_CRIT(...)		::Haketon::Log::PrintMessage(::Haketon::Log::Type::Client, ::Haketon::Log::Level::Fatal, __VA_ARGS__)

// TODO: Support custom LogCategories like UE4

namespace Haketon
{
	template <typename... Args>
	void Log::PrintMessage(Log::Type type, Log::Level level, fmt::format_string<Args...> fmt, Args&&... args)
	{
		Impl_PrintMessage(type, level, fmt::format(fmt, std::forward<Args>(args)...));
	}

	inline void Log::PrintMessage(Log::Type type, Log::Level level, std::string_view message)
	{
		Impl_PrintMessage(type, level, message);
	}
	
	template <typename... Args>
	void Log::PrintAssertMessage(Log::Type type, std::string_view prefix, fmt::format_string<Args...> fmt, Args&&... args)
	{
		std::string message = fmt::format(fmt, std::forward<Args>(args)...);
		Impl_PrintMessage(type, Log::Level::Error, fmt::format("{}{}", prefix, message));
	}

	inline void Log::PrintAssertMessage(Log::Type type, std::string_view prefix, std::string_view message)
	{
		Impl_PrintMessage(type, Log::Level::Error, fmt::format("{}{}", prefix, message));
	}
}