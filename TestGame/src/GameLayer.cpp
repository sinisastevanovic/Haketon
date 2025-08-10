#include "GameLayer.h"

#include "Haketon/Events/SceneEvents.h"

GameLayer::GameLayer()
    : Layer("GameLayer")
{
}

void GameLayer::OnAttach()
{
    HK_PROFILE_FUNCTION();
    
    /*Haketon::FramebufferSpecification fbSpec;
    fbSpec.Attachments = { Haketon::FramebufferTextureFormat::RGBA8, Haketon::FramebufferTextureFormat::Depth };
    fbSpec.Width = 1280;
    fbSpec.Height = 720;
    m_Framebuffer = Haketon::Framebuffer::Create(fbSpec);*/

    /*m_Scene = Haketon::CreateRef<Haketon::Scene>();*/
    
    // Load startup scene if specified

    
    /*m_Scene->OnViewportResize(1280, 720);*/

    HK_INFO("GameLayer::OnAttach");
}

void GameLayer::OnDetach()
{
    HK_PROFILE_FUNCTION();

    HK_INFO("GameLayer::OnDetach");
}

void GameLayer::OnUpdate(Haketon::Timestep ts)
{
    HK_PROFILE_FUNCTION();

    HK_INFO("GameLayer::OnUpdate");
    
    // Update scene
    /*m_Scene->OnUpdateRuntime(ts);*/
}

void GameLayer::OnEvent(Haketon::Event& e)
{
    // Handle events here
    Haketon::EventDispatcher dispatcher(e);
    dispatcher.Dispatch<Haketon::ScenePauseEvent>(HK_BIND_EVENT_FN(OnScenePauseEvent));
}

bool GameLayer::OnScenePauseEvent(Haketon::ScenePauseEvent& e)
{
    return false;
}
