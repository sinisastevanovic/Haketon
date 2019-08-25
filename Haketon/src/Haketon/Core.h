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