#pragma once

#include "Haketon.h"
#include "Haketon/Core/Layer.h"

class ExampleLayer : public Haketon::Layer
{
public:
    ExampleLayer();
    virtual ~ExampleLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    
    virtual void OnUpdate(Haketon::Timestep ts) override;
    virtual void OnEvent(Haketon::Event& e) override;
    virtual void OnImGuiRender() override;

private:
    Haketon::ShaderLibrary m_ShaderLibrary;
    Haketon::Ref<Haketon::Shader> m_TriangleShader;
    Haketon::Ref<Haketon::VertexArray> m_TriVertexArray;

    Haketon::Ref<Haketon::Shader> m_flatColorShader;
    Haketon::Ref<Haketon::VertexArray> m_SquareVertexArray;

    Haketon::Ref<Haketon::Texture2D> m_Texture;
    Haketon::Ref<Haketon::Texture2D> m_LogoTexture;

    Haketon::OrthographicCameraController m_CameraController;

    Haketon::FVec3 m_Color1 = { 0.8f, 0.2f, 0.3f };
    Haketon::FVec3 m_Color2 = { 0.2f, 0.3f, 0.8f };
};