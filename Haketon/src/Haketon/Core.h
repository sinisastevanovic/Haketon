#pragma once

#ifdef HK_PLATFORM_WINDOWS
	#ifdef HK_BUILD_DLL
		#define HAKETON_API __declspec(dllexport)
	#else 
		#define HAKETON_API __declspec(dllimport)
	#endif
#else
	#error Haketon only supports Windows!
#endif

#ifdef HK_ENABLE_ASSERTS
	#define HK_ASSERT(x, ...) { if(!(x)) { HK_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define HK_CORE_ASSERT(x, ...) { if(!(x)) { HK_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define HZ_ASSERT(x, ...)
	#define HK_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)