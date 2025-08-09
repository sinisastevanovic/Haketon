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

class TestGame : public Haketon::Application
{
public:
    TestGame(Haketon::ApplicationCommandLineArgs args)
        : Application("TestGame", args, false)
    {
        // Initialize your game here
        PushLayer(new GameLayer());
    }

    ~TestGame()
    {
    }
};

#ifdef GAME_DLL
extern "C" GAME_API Haketon::Application* CreateApplication(Haketon::ApplicationCommandLineArgs args)
{
    return new TestGame(args);
}
#else
#include <Haketon/Core/EntryPoint.h>

Haketon::Application* Haketon::CreateApplication(
    Haketon::ApplicationCommandLineArgs args)
{
    return new TestGame(args);
}
#endif


