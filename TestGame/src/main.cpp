#include <Haketon.h>
#include "GameLayer.h"
#include "Haketon/Core/IApplicationContext.h"

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
        HK_CORE_INFO("Test");
    }
};

class GameModuleContext : public Haketon::IApplicationContext
{
    // we could define some game specific data here
};

#ifdef GAME_DLL

extern "C" {

    GAME_API Haketon::IApplicationContext* AttachGameToHost(Haketon::Application* hostApp)
    {
        auto* context = new GameModuleContext();

        auto* gameLayer = new GameLayer();
        hostApp->PushLayer(gameLayer);
        context->CreatedLayers.push_back(gameLayer);

        return context;
    }

    GAME_API void DetachGameFromHost(Haketon::IApplicationContext* context)
    {
        delete context;
    }
}

#else
#include <Haketon/Core/EntryPoint.h>

Haketon::Application* Haketon::CreateApplication(
    Haketon::ApplicationCommandLineArgs args)
{
    return new TestGame(args);
}
#endif


