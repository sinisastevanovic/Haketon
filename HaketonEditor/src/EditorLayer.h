﻿#pragma once

#include "Haketon.h"
#include "Panels/Console.h"
#include "Panels/SceneHierarchyPanel.h"

#include "Haketon/Renderer/EditorCamera.h"

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
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
        bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);

        void NewScene();
        void OpenScene();
        void SaveScene();
        void SaveSceneAs();
    
    private:
        
        Ref<Scene> m_ActiveScene;
        Entity m_SquareEntity;
        Entity m_CameraEntity;

        Entity m_HoveredEntity;
        
        EditorCamera m_EditorCamera;

        // TODO: Remove this
        Ref<Texture2D> m_Texture;       
        Ref<Framebuffer> m_Framebuffer;

        bool m_ViewportFocused = false, m_ViewportHovered = false;
        FVec2 m_ViewportSize = { 0.0f, 0.0f };
        FVec2 m_ViewportBounds[2];

        int m_GizmoType = 0;

        // Panels
        SceneHierarchyPanel m_SceneHierarchyPanel;
        Console m_Console;

        
    };
    
    
}

