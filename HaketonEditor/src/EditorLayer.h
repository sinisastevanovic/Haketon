#pragma once

#include "Haketon.h"
#include "Haketon/Core/Layer.h"
#include "Haketon/Renderer/Texture.h"
#include "Haketon/Renderer/Framebuffer.h"

namespace Haketon
{
    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
    
        virtual void OnUpdate(Timestep ts) override;
        virtual void OnEvent(Event& e) override;
        virtual void OnImGuiRender() override;

    private:
        OrthographicCameraController m_CameraController;

        // TODO: Remove this
        Ref<Texture2D> m_Texture;       
        Ref<Framebuffer> m_Framebuffer;

        bool m_ViewportFocused = false, m_ViewportHovered = false;
        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
    };
}

