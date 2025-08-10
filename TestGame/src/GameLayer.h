#pragma once

#include <Haketon.h>


namespace Haketon
{
    class ScenePauseEvent;
}

class GameLayer : public Haketon::Layer
{
public:
    GameLayer();
    virtual ~GameLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(Haketon::Timestep ts) override;
    virtual void OnEvent(Haketon::Event& e) override;

private:
    bool OnScenePauseEvent(Haketon::ScenePauseEvent& e);
    
private:
    Haketon::Ref<Haketon::Scene> m_Scene;
    Haketon::Ref<Haketon::Framebuffer> m_Framebuffer;
};
