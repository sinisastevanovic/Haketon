#include "EditorLayer.h"
#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

//#include "Haketon/Scene/SceneSerializer.h"
#include <rttr/type>
#include <entt/entt.hpp>


#include "Haketon/Core/Misc/UUID.h"
#include "Haketon/Scene/SceneCamera.h"
#include "Haketon/Scene/Components/CameraComponent.h"
#include "imgui_internal.h"

#include "Haketon/Utils/PlatformUtils.h"
#include "ImGuizmo.h"

#include "Haketon/Math/Math.h"
#include "Haketon/Scene/Components/TagComponent.h"
#include "Haketon/Scene/Components.h"
#include <filesystem>

#include "Haketon/Core/Serialization/RapidJsonDeserializer.h"
#include "Haketon/Core/Serialization/RapidJsonSerializer.h"
#include "Haketon/Events/SceneEvents.h"

static rttr::string_view library_name("Haketon");


namespace Haketon
{
	EditorLayer::EditorLayer()
        : Layer("EditorLayer")
	{
		m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
	}

	void EditorLayer::OnAttach()
	{
		HK_PROFILE_FUNCTION();

		m_Texture = Texture2D::Create();
		
		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_EditorCamera = EditorCamera(30.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
		
		m_SceneHierarchyPanel.SetContext(Application::Get().GetActiveScene());
	}

	void EditorLayer::OnDetach()
	{
		HK_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		HK_PROFILE_FUNCTION();


		// Resize
		if(FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			Application::Get().GetActiveScene()->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		if(m_ViewportHovered)
			m_EditorCamera.OnUpdate(ts);

		Renderer2D::ResetStats();

		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();
		
		m_Framebuffer->ClearAttachment(1, -1);

		// Update Scene
		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				Application::Get().GetActiveScene()->OnUpdateEditor(ts, m_EditorCamera);
				break;
			}
			case SceneState::Play:
			case SceneState::Pause:
			{
				Application::Get().GetActiveScene()->OnUpdateRuntime(ts);
				break;
			}
		}

		auto [MX, MY] = ImGui::GetMousePos();
		MX -= m_ViewportBounds[0].x;
		MY -= m_ViewportBounds[0].y;
		FVec2 ViewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		MY = ViewportSize.y - MY;

		int MouseX = (int)MX;
		int MouseY = (int)MY;

		if(MouseX >= 0 && MouseY >= 0 && MouseX < (int)ViewportSize.x && MouseY < (int)ViewportSize.y)
		{
			int PixelData = m_Framebuffer->ReadPixel(1, MouseX, MouseY);
			m_HoveredEntity = PixelData == -1 ? Entity() : Entity((entt::entity)PixelData, Application::Get().GetActiveScene());
		}
		
		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher Dispatcher(e);
		Dispatcher.Dispatch<KeyPressedEvent>(HK_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		Dispatcher.Dispatch<MouseButtonPressedEvent>(HK_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
		Dispatcher.Dispatch<MouseButtonReleasedEvent>(HK_BIND_EVENT_FN(EditorLayer::OnMouseButtonReleased));
		Dispatcher.Dispatch<ActiveSceneChangedEvent>(HK_BIND_EVENT_FN(EditorLayer::OnActiveSceneChanged));
	}

	void EditorLayer::OnImGuiRender()
	{
		HK_PROFILE_FUNCTION();

		/*static bool show = true;
		ImGui::ShowDemoWindow(&show);*/

		// Set this to true to enable dockspace
		static bool dockingEnabled = true;
		if(dockingEnabled)
		{
			static bool dockspaceOpen = true;
		    static bool opt_fullscreen = true;
		    static bool opt_padding = false;
		    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		    // because it would be confusing to have two docking targets within each others.
		    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		    if (opt_fullscreen)
		    {
		        ImGuiViewport* viewport = ImGui::GetMainViewport();
		        ImGui::SetNextWindowPos(viewport->WorkPos);
		        ImGui::SetNextWindowSize(viewport->WorkSize);
		        ImGui::SetNextWindowViewport(viewport->ID);
		        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		    }
		    else
		    {
		        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		    }

		    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		    // and handle the pass-thru hole, so we ask Begin() to not render a background.
		    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		        window_flags |= ImGuiWindowFlags_NoBackground;

		    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		    // all active windows docked into it will lose their parent and become undocked.
		    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		    if (!opt_padding)
		        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		    if (!opt_padding)
		        ImGui::PopStyleVar();

		    if (opt_fullscreen)
		        ImGui::PopStyleVar(2);

		    // DockSpace
		    ImGuiIO& io = ImGui::GetIO();
			ImGuiStyle& style = ImGui::GetStyle();
			float minWinSizeX = style.WindowMinSize.x;
			style.WindowMinSize.x = 370.0f;
		    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		    {
		        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		    }

			style.WindowMinSize.x = minWinSizeX;
			
		    if (ImGui::BeginMenuBar())
		    {
		        if (ImGui::BeginMenu("File"))
		        {
		        	if (ImGui::BeginMenu("Project"))
		        	{
		        		if (ImGui::MenuItem("New Project..."))
		        		{
		        			NewProject();
		        		}
		        		
		        		if (ImGui::MenuItem("Open Project..."))
		        		{
		        			OpenProject();
		        		}
		        		
		        		if (ImGui::MenuItem("Save Project", nullptr, false, m_CurrentProject != nullptr))
		        		{
		        			SaveProject();
		        		}
		        		
		        		ImGui::Separator();
		        		
		        		if (ImGui::MenuItem("Build Project", nullptr, false, m_CurrentProject != nullptr))
		        		{
		        			BuildProject();
		        		}
		        		
		        		ImGui::EndMenu();
		        	}
		        	
		        	ImGui::Separator();
		        	
		        	if (ImGui::MenuItem("New Scene", "Ctrl+N"))
		        	{
		        		NewScene();
		        	}
		        	
		            if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
		            {
		        		OpenScene();
		            }

		        	if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
		        	{
		        		SaveScene();
		        	}
		        	
		        	if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
		        	{
		        		SaveSceneAs();
		        	}

		        	ImGui::Separator();
		        	
		        	if (ImGui::MenuItem("Exit")) { GetApplication().Close(); }     
		            ImGui::EndMenu();
		        }
		        ImGui::EndMenuBar();
		    }

			m_SceneHierarchyPanel.OnImGuiRender();

			ImGui::Begin("Stats");

			std::string Name = "None";
			if(m_HoveredEntity)
				Name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
			ImGui::Text("Hovered Entity: %s", Name.c_str());
			
			auto stats = Renderer2D::GetStats();
			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Quad Count: %d", stats.QuadCount);
			ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
			ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
			
			ImGui::End();			

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
			ImGui::Begin("Viewport");

			auto ViewportMinRegion = ImGui::GetWindowContentRegionMin();
			auto ViewportMaxRegion = ImGui::GetWindowContentRegionMax();
			auto ViewportOffset = ImGui::GetWindowPos(); // Includes tab bar
			m_ViewportBounds[0] = { ViewportMinRegion.x + ViewportOffset.x, ViewportMinRegion.y + ViewportOffset.y };
			m_ViewportBounds[1] = { ViewportMaxRegion.x + ViewportOffset.x, ViewportMaxRegion.y + ViewportOffset.y };

			m_ViewportFocused = ImGui::IsWindowFocused();
			m_ViewportHovered = ImGui::IsWindowHovered();
			//Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered); // TODO: Handle this differently.. Sucks if we are writing in a Textbox. And Shortcuts don't work if not on Viewport...
			Application::Get().GetImGuiLayer()->SetBlockEvents(false);
			
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();		
			m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

			uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
			ImGui::Image(textureID, viewportPanelSize, ImVec2{ 0, 1 }, ImVec2{1, 0});
			

			// Gizmos
			Entity SelectedEntity = m_SceneHierarchyPanel.GetSelectedEntity(); // TODO: When mouse picking is implemented, remove this!
			if(SelectedEntity && m_GizmoType != -1)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
				ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

				// Camera				

				// Runtime camera from entity
				/*auto CameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
				const auto& Camera = CameraEntity.GetComponent<CameraComponent>().Camera;
				const glm::mat4& CameraProjection = Camera->GetProjection();
				glm::mat4 CameraView = glm::inverse(CameraEntity.GetComponent<TransformComponent>().GetTransform());*/

				// Entity Transform
				auto& TransformComp = SelectedEntity.GetComponent<TransformComponent>();
				glm::mat4 Transform = TransformComp.GetTransform();

				// Snapping
				bool Snap = Input::IsKeyPressed(Key::LeftControl);
				float SnapValue = m_GizmoType == ImGuizmo::OPERATION::ROTATE ? 45.0f : 0.5f; // TODO: Add UI for Gizmo Settings
				float SnapValues[3] = { SnapValue, SnapValue, SnapValue };

				// Editor Camera
				if (m_SceneState == SceneState::Edit)
				{
					const glm::mat4& CameraProjection = m_EditorCamera.GetProjection();
					glm::mat4 CameraView = m_EditorCamera.GetViewMatrix();
					
					ImGuizmo::Manipulate(glm::value_ptr(CameraView), glm::value_ptr(CameraProjection),
						(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(Transform),
						nullptr, Snap ? SnapValues : nullptr);
				}
				else
				{
					auto CameraEntity = Application::Get().GetActiveScene()->GetPrimaryCameraEntity();
					const auto& primaryCamera = CameraEntity.GetComponent<CameraComponent>();
					const glm::mat4& CameraProjection = primaryCamera.Camera->GetProjection();
					glm::mat4 CameraView = glm::inverse(CameraEntity.GetComponent<TransformComponent>().GetTransform());
					
					ImGuizmo::Manipulate(glm::value_ptr(CameraView), glm::value_ptr(CameraProjection),
						(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(Transform),
						nullptr, Snap ? SnapValues : nullptr);
				}
				
				if(ImGuizmo::IsUsing())
				{
					FVec3 Translation, Rotation, Scale;
					Math::DecomposeTransform(Transform, Translation, Rotation, Scale);

					FVec3 DeltaRotation = Rotation - TransformComp.Rotation;
					TransformComp.Position = Translation;
					TransformComp.Rotation += DeltaRotation;
					TransformComp.Scale = Scale;
				}
			}

			// Play/Stop Controls
			float buttonSize = 30.0f;
			ImGui::SetCursorPos(ImVec2{viewportPanelSize.x * 0.5f - 50, buttonSize});
			if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Pause)
			{
				if (ImGui::Button("Play", ImVec2(50, buttonSize)))
					OnScenePlay();
			}
			else if (m_SceneState == SceneState::Play)
			{
				if (ImGui::Button("Pause", ImVec2(50, buttonSize)))
					OnScenePause();
			}
			
			ImGui::SameLine();
			if (ImGui::Button("Stop", ImVec2(50, buttonSize)) && m_SceneState != SceneState::Edit)
				OnSceneStop();

			ImGui::SetCursorPos(ImVec2{(viewportPanelSize.x - 3 * buttonSize) - 5, buttonSize});
			if(ImGui::Button("W", ImVec2(buttonSize, buttonSize)) && !ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			ImGui::SetCursorPos(ImVec2{(viewportPanelSize.x - 2 * buttonSize) - 5, buttonSize});
			if(ImGui::Button("E", ImVec2(buttonSize, buttonSize)) && !ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			ImGui::SetCursorPos(ImVec2{(viewportPanelSize.x - buttonSize) - 5, buttonSize});
			if(ImGui::Button("R", ImVec2(buttonSize, buttonSize)) && !ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
			
			ImGui::End();
			ImGui::PopStyleVar();
			
		    ImGui::End();

			bool ShowConsole = true;
			m_Console.Draw("Console", &ShowConsole);
			
			//ImGui::ShowDemoWindow();
		}
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if(e.GetRepeatCount() > 0)
			return false;

		bool CtrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool ShiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		
		switch(e.GetKeyCode())
		{			
			case Key::N:
			{
				if(CtrlPressed)
					NewScene();
				break;
			}
			case Key::O:
			{
				if(CtrlPressed)
					OpenScene();
				break;
			}
			case Key::S:
			{
				if(CtrlPressed && ShiftPressed)
					SaveSceneAs();
				else if(CtrlPressed)
					SaveScene();
					
				break;
			}

			// Gizmos
			case Key::Q:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = -1;
				break;
			}
			case Key::W:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			}
			case Key::E:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			}
			case Key::R:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			}
			
			default: ;
		}

		return true;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		return true;
	}

	bool EditorLayer::OnMouseButtonReleased(MouseButtonReleasedEvent& e)
	{
		// Mouse picking
		if(e.GetMouseButton() == Mouse::ButtonLeft)
		{
			if(m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
				m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
		}

		return true;
	}

	bool EditorLayer::OnActiveSceneChanged(ActiveSceneChangedEvent& e)
	{
		Scene* activeScene = Application::Get().GetActiveScene();
		activeScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(activeScene);
		return false;
	}

	void EditorLayer::NewScene()
	{
		SceneNewEvent e;
		Event::Dispatch(e);
	}

	void EditorLayer::OpenScene()
	{
		std::string filePath = FileDialogs::OpenFile("Haketon Scene (*.haketon)\0*.haketon\0");
		OpenScene(filePath);
	}

	void EditorLayer::OpenScene(const std::string& path)
	{
		SceneOpenEvent e(path);
		Event::Dispatch(e);
	}

	void EditorLayer::SaveScene()
	{
		SceneSaveEvent e;
		Event::Dispatch(e);
	}

	void EditorLayer::SaveSceneAs()
	{
		SceneSaveAsEvent e;
		Event::Dispatch(e);
	}

	void EditorLayer::OnScenePlay()
	{
		if (m_SceneState == SceneState::Edit)
		{
			ScenePlayEvent event;
			Event::Dispatch(event);
			
			m_SceneState = SceneState::Play;
		}
		else if (m_SceneState == SceneState::Pause)
		{
			ScenePauseEvent event(false);
			Event::Dispatch(event);

			m_SceneState = SceneState::Play;
		}
	}

	void EditorLayer::OnSceneStop()
	{
		if (m_SceneState == SceneState::Play || m_SceneState == SceneState::Pause)
		{
			SceneStopEvent event;
			Event::Dispatch(event);
			m_SceneState = SceneState::Edit;
		}
	}

	void EditorLayer::OnScenePause()
	{
		if (m_SceneState == SceneState::Play)
		{
			ScenePauseEvent event(true);
			Event::Dispatch(event);
			m_SceneState = SceneState::Pause;
		}
	}

	void EditorLayer::NewProject()
	{
		std::string projectPath = FileDialogs::SaveFile("Haketon Project (*.hkproject)\0*.hkproject\0");
		if (!projectPath.empty())
		{
			std::string projectName = std::filesystem::path(projectPath).stem().string();
			m_CurrentProject = Project::New(projectPath, projectName);
			
			if (m_CurrentProject)
			{
				HK_CORE_INFO("Created new project: {0}", projectName);
			}
		}
	}

	void EditorLayer::OpenProject()
	{
		std::string projectPath = FileDialogs::OpenFile("Haketon Project (*.hkproject)\0*.hkproject\0");
		if (!projectPath.empty())
		{
			m_CurrentProject = Project::Load(projectPath);
			
			if (m_CurrentProject)
			{
				GetApplication().SetWindowTitle("Haketon Editor - " + m_CurrentProject->GetConfig().Name);
				HK_CORE_INFO("Opened project: {0}", m_CurrentProject->GetConfig().Name);

				OpenScene(m_CurrentProject->GetAssetDirectory() + "/scenes/" + m_CurrentProject->GetConfig().StartupScene);
			}
		}
	}

	void EditorLayer::SaveProject()
	{
		if (m_CurrentProject)
		{
			if (m_CurrentProject->Save())
			{
				HK_CORE_INFO("Project saved successfully");
			}
			else
			{
				HK_CORE_ERROR("Failed to save project");
			}
		}
	}

	void EditorLayer::BuildProject()
	{
		if (m_CurrentProject)
		{
			HK_CORE_INFO("Building project: {0}", m_CurrentProject->GetConfig().Name);
			
			if (m_CurrentProject->Build())
			{
				HK_CORE_INFO("Project built successfully");
			}
			else
			{
				HK_CORE_ERROR("Project build failed");
			}
		}
	}
}

