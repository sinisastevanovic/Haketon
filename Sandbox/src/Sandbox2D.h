﻿#pragma once

#include "Haketon.h"
#include "Haketon/Core/Layer.h"
#include "Haketon/Renderer/Texture.h"
#include "ParticleSystem.h"
#include "Haketon/Renderer/Framebuffer.h"

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
    Haketon::Ref<Haketon::Texture2D> m_SpriteSheet;
    Haketon::Ref<Haketon::SubTexture2D> m_WaterSubTexture;
    Haketon::Ref<Haketon::SubTexture2D> m_DirtSubTexture;
    
    glm::vec4 m_TextureTint = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec3 TexQuadPosition = {0.0f, 0.0f, 0.1f};
    FVec2 TexQuadScale = {1.0f, 1.0f};
    float TexQuadRotation = 0.0f;
    float TexQuadTiling = 1.0f;

    ParticleProps m_Particle;
    ParticleSystem m_ParticleSystem;

    uint32_t m_MapWidth, m_MapHeight;
    std::unordered_map<char, Haketon::Ref<Haketon::SubTexture2D>> m_TextureMap;
};
