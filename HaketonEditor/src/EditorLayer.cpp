#include "EditorLayer.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

//#include "Haketon/Scene/SceneSerializer.h"
#include <rttr/type>
#include <entt/include/entt.hpp>


#include "Haketon/Core/Serializer.h"
#include "Haketon/Core/Misc/Guid.h"
#include "Haketon/Scene/SceneCamera.h"
#include "Haketon/Scene/Components/CameraComponent.h"
#include "imgui/imgui_internal.h"

#include "Haketon/Utils/PlatformUtils.h"
#include "ImGuizmo/ImGuizmo.h"

#include "Haketon/Math/Math.h"
#include "Haketon/Scene/Components/TagComponent.h"

static rttr::string_view library_name("Haketon");


namespace Haketon
{
	EditorLayer::EditorLayer()
        : Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f, true)
	{
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

		m_ActiveScene = CreateRef<Scene>();

		auto commandLineArgs = Application::Get().GetCommandLineArgs();
		if(commandLineArgs.Count > 1)
		{
			auto sceneFilePath = commandLineArgs[1];
			Serializer::DeserializeSceneFromFile(sceneFilePath, m_ActiveScene);
		}

		m_EditorCamera = EditorCamera(30.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
#if 0
		m_SquareEntity = m_ActiveScene->CreateEntity("Square");
		m_SquareEntity.AddComponent<SpriteRendererComponent>(glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});

		auto square2 = m_ActiveScene->CreateEntity("Square2");
		square2.AddComponent<SpriteRendererComponent>(glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
		auto& cameraComp = m_CameraEntity.AddComponent<CameraComponent>();
		cameraComp.Primary = true;

		auto camera2 = m_ActiveScene->CreateEntity("Camera2");
		camera2.AddComponent<CameraComponent>();

		class CameraController : public ScriptableEntity
		{
		public:
	
			void OnUpdate(Timestep ts)
			{
				auto& tc = GetComponent<TransformComponent>();
				float speed = 5.0f;

				if(Input::IsKeyPressed(Key::W))
					tc.Position.y += speed * ts;
				if(Input::IsKeyPressed(Key::S))
					tc.Position.y -= speed * ts;
				if(Input::IsKeyPressed(Key::A))
					tc.Position.x -= speed * ts;
				if(Input::IsKeyPressed(Key::D))
					tc.Position.x += speed * ts;
			}			
		};

		m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
#endif

		/*m_CameraEntity = m_ActiveScene->CreateEntity("Entity1");
		auto& comp = m_CameraEntity.AddComponent<CameraComponent>();
		comp.Primary = true;*/

		auto spriteEntity = m_ActiveScene->CreateEntity("Sprite");
		spriteEntity.AddComponent<SpriteRendererComponent>();
		
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
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
			m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		if(m_ViewportFocused)
		{
			m_CameraController.OnUpdate(ts); // TODO: Do we need a camera controller??
		}

		if(m_ViewportHovered)
			m_EditorCamera.OnUpdate(ts);

		Renderer2D::ResetStats();

		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();
		
		m_Framebuffer->ClearAttachment(1, -1);

		// Update Scene
		m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);

		auto [MX, MY] = ImGui::GetMousePos();
		MX -= m_ViewportBounds[0].x;
		MY -= m_ViewportBounds[0].y;
		glm::vec2 ViewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		MY = ViewportSize.y - MY;

		int MouseX = (int)MX;
		int MouseY = (int)MY;

		if(MouseX >= 0 && MouseY >= 0 && MouseX < (int)ViewportSize.x && MouseY < (int)ViewportSize.y)
		{
			int PixelData = m_Framebuffer->ReadPixel(1, MouseX, MouseY);
			m_HoveredEntity = PixelData == -1 ? Entity() : Entity((entt::entity)PixelData, m_ActiveScene.get());
		}
		
		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnEvent(Event& e)
	{
	    m_CameraController.OnEvent(e);
		m_EditorCamera.OnEvent(e);

		EventDispatcher Dispatcher(e);
		Dispatcher.Dispatch<KeyPressedEvent>(HK_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		Dispatcher.Dispatch<MouseButtonPressedEvent>(HK_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
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
		        	if (ImGui::MenuItem("New", "Ctrl+N"))
		        	{
		        		NewScene();
		        	}
		        	
		            if (ImGui::MenuItem("Open...", "Ctrl+O"))
		            {
		        		OpenScene();
		            }

		        	if (ImGui::MenuItem("Save", "Ctrl+S"))
		        	{
		        		SaveScene();
		        	}
		        	
		        	if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
		        	{
		        		SaveSceneAs();
		        	}

		        	if (ImGui::MenuItem("Exit")) { Application::Get().Close(); }     
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
			ImGui::Image(reinterpret_cast<void*>(textureID), viewportPanelSize, ImVec2{ 0, 1 }, ImVec2{1, 0});

			// Gizmos
			Entity SelectedEntity = m_SceneHierarchyPanel.GetSelectedEntity(); // TODO: When mouse picking is implemented, remove this!
			if(SelectedEntity && m_GizmoType != -1)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

				// Camera				

				// Runtime camera from entity
				/*auto CameraEntity = m_ActiveScene->GetPrimaryCameraEntity(); // TODO: Implement a editor camera
				const auto& Camera = CameraEntity.GetComponent<CameraComponent>().Camera;
				const glm::mat4& CameraProjection = Camera->GetProjection();
				glm::mat4 CameraView = glm::inverse(CameraEntity.GetComponent<TransformComponent>().GetTransform());*/

				// Editor Camera
				const glm::mat4& CameraProjection = m_EditorCamera.GetProjection();
				glm::mat4 CameraView = m_EditorCamera.GetViewMatrix();

				// Entity Transform
				auto& TransformComp = SelectedEntity.GetComponent<TransformComponent>();
				glm::mat4 Transform = TransformComp.GetTransform();

				// Snapping
				bool Snap = Input::IsKeyPressed(Key::LeftControl);
				float SnapValue = m_GizmoType == ImGuizmo::OPERATION::ROTATE ? 45.0f : 0.5f; // TODO: Add UI for Gizmo Settings
				float SnapValues[3] = { SnapValue, SnapValue, SnapValue };
				
				ImGuizmo::Manipulate(glm::value_ptr(CameraView), glm::value_ptr(CameraProjection),
					(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(Transform),
					nullptr, Snap ? SnapValues : nullptr);

				if(ImGuizmo::IsUsing())
				{
					glm::vec3 Translation, Rotation, Scale;
					Math::DecomposeTransform(Transform, Translation, Rotation, Scale);

					glm::vec3 DeltaRotation = Rotation - TransformComp.Rotation;
					TransformComp.Position = Translation;
					TransformComp.Rotation += DeltaRotation;
					TransformComp.Scale = Scale;
				}
			}

			
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
		// Mouse picking
		if(e.GetMouseButton() == Mouse::ButtonLeft)
		{
			if(m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
				m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
		}

		return true;
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OpenScene()
	{
		std::string filePath = FileDialogs::OpenFile("Haketon Scene (*.haketon)\0*.haketon\0");
		if(!filePath.empty())
		{
			m_ActiveScene = CreateRef<Scene>();
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		            		
			Serializer::DeserializeSceneFromFile(filePath, m_ActiveScene);
		}
	}

	void EditorLayer::SaveScene()
	{
		// TODO: Implement SaveScene()
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filePath = FileDialogs::SaveFile("Haketon Scene (*.haketon)\0*.haketon\0");
		if(!filePath.empty())
		{
			Serializer::SerializeScene(m_ActiveScene, filePath);
		}
	}
}

