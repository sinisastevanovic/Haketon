#include "hkpch.h"

#include "Haketon/Core/Window.h"

#ifdef HK_PLATFORM_WINDOWS
    #include "Platform/Windows/WindowsWindow.h"
#endif

namespace Haketon
{
    Scope<Window> Window::Create(const WindowProps& props)
    {
#ifdef HK_PLATFORM_WINDOWS
        return CreateScope<WindowsWindow>(props);
#else
        HK_CORE_ASSERT(false, "Unknown platform!");
        return nullptr;
#endif
    }
}