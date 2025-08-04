#pragma once

#include "Haketon.h"
#include "Panels/Console.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Project/Project.h"

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
        void OpenScene(const std::string& path);
        void SaveScene();
        void SaveSceneAs();
        
        void OnScenePlay();
        void OnSceneStop();
        void OnScenePause();
        
        void NewProject();
        void OpenProject();
        void SaveProject();
        void BuildProject();
    
    private:
        enum class SceneState
        {
            Edit = 0, Play = 1, Pause = 2
        };
        SceneState m_SceneState = SceneState::Edit;
        
        Ref<Scene> m_ActiveScene;
        Ref<Scene> m_EditorScene;
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
        
        // Project
        Ref<Project> m_CurrentProject;

        
    };
    
    
}

