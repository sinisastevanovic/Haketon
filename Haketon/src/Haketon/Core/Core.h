#pragma once

#include <memory>

#ifdef HK_PLATFORM_WINDOWS

#else
	#error Haketon only supports Windows!
#endif

#ifdef HK_DEBUG
	#define HK_ENABLE_ASSERTS
#endif

#ifdef HK_ENABLE_ASSERTS
	#define HK_ASSERT(x, ...) { if(!(x)) { HK_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define HK_CORE_ASSERT(x, ...) { if(!(x)) { HK_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define HZ_ASSERT(x, ...)
	#define HK_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define HK_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

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