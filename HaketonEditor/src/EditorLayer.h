#pragma once

#include "Haketon.h"
#include "Panels/Console.h"
#include "Panels/SceneHierarchyPanel.h"


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

        Ref<Scene> m_ActiveScene;
        Entity m_SquareEntity;
        Entity m_CameraEntity;

        // TODO: Remove this
        Ref<Texture2D> m_Texture;       
        Ref<Framebuffer> m_Framebuffer;

        bool m_ViewportFocused = false, m_ViewportHovered = false;
        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

        // Panels
        SceneHierarchyPanel m_SceneHierarchyPanel;
        Console m_Console;
    };
    
    
}

