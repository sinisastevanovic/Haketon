#include "hkpch.h"
#include "Haketon/Core/Input.h"

#ifdef HK_PLATFORM_WINDOWS
    #include "Platform/Windows/WindowsInput.h"
#endif

namespace Haketon
{
    Scope<Input> Input::s_Instance = Input::Create();

    Scope<Input> Input::Create()
    {
    #ifdef HK_PLATFORM_WINDOWS
        return CreateScope<WindowsInput>();
    #else
        HK_CORE_ASSERT(false, "Unknown platform!");
        return nullptr;
    #endif
    }
}