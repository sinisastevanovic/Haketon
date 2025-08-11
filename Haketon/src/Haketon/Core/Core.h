#pragma once

#include <memory>

#define HK_STRINGIFY_MACRO(x) #x

#ifdef _WIN32
	#if defined(HK_ENGINE_DLL)
		#define HK_API __declspec(dllexport)
	#elif defined(HK_ENGINE_DLL_IMPORT) 
		#define HK_API __declspec(dllimport)
	#else
		#define HK_API
	#endif
#else
	#define HK_API
#endif

#define BIT(x) (1 << x)

#define HK_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define CLASS(...)
#define STRUCT(...)
#define ENUM(...)
#define PROPERTY(...)
#define FUNCTION(...)

namespace Haketon
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}

#include "Haketon/Core/Log.h"
#include "Haketon/Core/Assert.h"