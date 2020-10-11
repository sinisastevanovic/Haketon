#pragma once

#include "Haketon.h"
#include "Haketon/Core/Layer.h"

class Sandbox2D : public Haketon::Layer
{
public:
    Sandbox2D();
    virtual ~Sandbox2D() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    
    virtual void OnUpdate(Haketon::Timestep ts) override;
    virtual void OnEvent(Haketon::Event& e) override;
    virtual void OnImGuiRender() override;

private:
    Haketon::OrthographicCameraController m_CameraController;

    // TODO: Remove this
    Haketon::Ref<Haketon::VertexArray> m_SquareVA;
    Haketon::Ref<Haketon::Shader> m_FlatColorShader;
    
    glm::vec4 m_Color = { 0.8f, 0.2f, 0.3f, 1.0f };
};
