#include <Haketon.h>
#include "GameLayer.h"

#ifdef _WIN32
    #ifdef GAME_DLL
        #define GAME_API __declspec(dllexport)
    #else
        #define GAME_API __declspec(dllimport)
    #endif
#else
    #define GAME_API
#endif

class TestGame : public Haketon::GameApplication
{
public:
    TestGame(Haketon::ApplicationCommandLineArgs args)
        : GameApplication("TestGame", args, false)
    {
        // Initialize your game here
        PushLayer(new GameLayer());
    }

    ~TestGame()
    {
        HK_CORE_INFO("Test");
    }
};

#ifdef GAME_DLL
extern "C" GAME_API Haketon::Application* CreateApplication(Haketon::ApplicationCommandLineArgs args)
{
    return new TestGame(args);
}

extern "C" GAME_API void DestroyApplication(Haketon::Application* app)
{
    delete app;
}
#else
#include <Haketon/Core/EntryPoint.h>

Haketon::Application* Haketon::CreateApplication(
    Haketon::ApplicationCommandLineArgs args)
{
    return new TestGame(args);
}
#endif


