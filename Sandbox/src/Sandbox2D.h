#pragma once

#include "Haketon.h"
#include "Haketon/Core/Layer.h"
#include "Haketon/Renderer/Texture.h"

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
    Haketon::Ref<Haketon::Texture2D> m_Texture;
    glm::vec4 m_TextureTint = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec3 TexQuadPosition = {0.0f, 0.0f, 0.0f};
    glm::vec2 TexQuadScale = {1.0f, 1.0f};
    float TexQuadRotation = 0.0f;
};
