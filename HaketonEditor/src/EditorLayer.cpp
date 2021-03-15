#include "EditorLayer.h"
#include "imgui/imgui.h"
#include "ReflectionRegistration.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Haketon/Scene/SceneSerializer.h"
#include <rttr/type>


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
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();
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

		m_CameraEntity = m_ActiveScene->CreateEntity("Entity1");
		m_CameraEntity.AddComponent<CameraComponent>();

		auto spriteEntity = m_ActiveScene->CreateEntity("Sprite");
		spriteEntity.AddComponent<SpriteRendererComponent>();

		
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		//SceneSerializer serializer(m_ActiveScene);
		//serializer.DeserializeText("assets/scenes/testscene.haketon");
	}

	void EditorLayer::OnDetach()
	{
		HK_PROFILE_FUNCTION();
		//SceneSerializer serializer(m_ActiveScene);
		//serializer.SerializeText("assets/scenes/testscene.haketon");
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

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		if(m_ViewportFocused)
			m_CameraController.OnUpdate(ts);	

		Renderer2D::ResetStats();

		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();

		// Update Scene
		m_ActiveScene->OnUpdate(ts);
		
		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnEvent(Event& e)
	{
	    m_CameraController.OnEvent(e);
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
		        ImGui::SetNextWindowPos(viewport->GetWorkPos());
		        ImGui::SetNextWindowSize(viewport->GetWorkSize());
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
		            if (ImGui::MenuItem("Serialize"))
		            {
		            	SceneSerializer serializer(m_ActiveScene);
		            	serializer.SerializeText("assets/scenes/testscene.haketon");
		            }

		        	if (ImGui::MenuItem("Deserialize"))
		        	{
		        		SceneSerializer serializer(m_ActiveScene);
		        		serializer.DeserializeText("assets/scenes/testscene.haketon");
		        	}

		        	if (ImGui::MenuItem("Exit")) { Application::Get().Close(); }     
		            ImGui::EndMenu();
		        }
		        ImGui::EndMenuBar();
		    }

			m_SceneHierarchyPanel.OnImGuiRender();

			ImGui::Begin("Stats");
			auto stats = Renderer2D::GetStats();
			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Quad Count: %d", stats.QuadCount);
			ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
			ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
			
			ImGui::End();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
			ImGui::Begin("Viewport");

			m_ViewportFocused = ImGui::IsWindowFocused();
			m_ViewportHovered = ImGui::IsWindowHovered();
			Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused || !m_ViewportHovered);
			
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();		
			m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

			uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
			ImGui::Image(reinterpret_cast<void*>(textureID), viewportPanelSize, ImVec2{ 0, 1 }, ImVec2{1, 0});
			ImGui::End();
			ImGui::PopStyleVar();
			
		    ImGui::End();
		}
	}
}

