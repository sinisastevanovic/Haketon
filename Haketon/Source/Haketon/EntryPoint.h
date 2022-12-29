#pragma once

#ifdef HK_PLATFORM_WINDOWS

extern Haketon::Application* Haketon::CreateApplication(int argc, char** argv);
bool g_ApplicationRunning = true;

namespace Haketon
{
    int Main(int argc, char** argv)
    {
        while (g_ApplicationRunning)
        {
            Haketon::Application* app = Haketon::CreateApplication(argc, argv);
            app->Run();
            delete app;
        }

        return 0;
    }
}

#ifdef HK_DIST

#include <Windows.h>

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    return Haketon::Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
    return Haketon::Main(argc, argv);
}

#endif // HK_DIST

#endif // HK_PLATFORM_WINDOWS