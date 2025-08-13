#include "hkpch.h"

#include "Application.h"

#include "imgui.h"
#include "Haketon/Events/Event.h"
#include "Log.h"
#include "Reflection.h"
#include "PathUtils.h"

#include "Haketon/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

#include "Asset/AssetManager.h"

namespace Haketon
{

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name, ApplicationCommandLineArgs args, bool maximized)
		: m_CommandLineArgs(args)
	{
		HK_PROFILE_FUNCTION();

		HK_CORE_ASSERT(!s_Instance, "Editor application already exists!");
		s_Instance = this;
		
		// Initialize path utils
		PathUtils::Initialize();
		
		// Initialize reflection system
		Reflection::Initialize();
		
		ModuleManager::Initialize();

		// We need to init the asset manager after the game has been loaded in editor
#ifndef HK_EDITOR
		AssetManager::Init();
#endif

		m_Window = Window::Create(WindowProps(name, maximized));
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		HK_PROFILE_FUNCTION();

		// Clean up layers first while systems are still available
		for (Layer* layer : m_LayerStack)
		{
			layer->OnDetach();
			delete layer;
		}
		m_LayerStack.ClearLayers();

		// Then shutdown systems
		Renderer::Shutdown();
#ifndef HK_EDITOR
		AssetManager::Shutdown();
#endif
		ModuleManager::Shutdown();
		Reflection::Shutdown();

		s_Instance = nullptr;
	}

	void Application::OnEvent(Event& e)
	{
		HK_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

		//HK_CORE_TRACE("{0}", e);
		
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::PushLayer(Layer* Layer)
	{
		HK_PROFILE_FUNCTION();

		Layer->SetApplication(this);
		m_LayerStack.PushLayer(Layer);
		Layer->OnAttach();
	}

	void Application::PushOverlay(Layer* Layer)
	{
		HK_PROFILE_FUNCTION();

		Layer->SetApplication(this);
		m_LayerStack.PushOverlay(Layer);
		Layer->OnAttach();
	}

	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack.PopLayer(layer);
	}

	void Application::PopOverlay(Layer* layer)
	{
		m_LayerStack.PopOverlay(layer);
	}

	void Application::Run()
	{
		HK_PROFILE_FUNCTION();

		while (m_Running)
		{
			HK_PROFILE_SCOPE("RunLoop");
			
			float time = (float)glfwGetTime(); // Todo: Replace with something like Platform::GetTime (abstraction)
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
			{
				{
					HK_PROFILE_SCOPE("LayerStack Updates");
					
					for (Layer* layer : m_LayerStack)	// That's sick
						layer->OnUpdate(timestep);	
				}		
			}

			m_ImGuiLayer->Begin();
			{
				HK_PROFILE_SCOPE("LayerStack OnImGuiRender");
				
				for (Layer* layer : m_LayerStack)	
					layer->OnImGuiRender();
			}			
			m_ImGuiLayer->End();
			
			m_Window->OnUpdate();

			RunImpl();
		}
	}

	Window& Application::GetWindow()
	{
		return *m_Window;
	}

	void Application::SetWindowTitle(const std::string& title)
	{
		m_Window->SetWindowTitle(title);
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::Shutdown()
	{
		// Clean up active scene
		m_ActiveScene.reset();
		
		// If shutdown is called during runtime (not from destructor), clean up layers
		if (!m_LayerStack.IsEmpty())
		{
			for (Layer* layer : m_LayerStack)
			{
				layer->OnDetach();
				delete layer;
			}
			m_LayerStack.ClearLayers();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		HK_PROFILE_FUNCTION();

		if(e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;

		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	void Application::ShareImGuiContext()
	{
		// For DLL builds, ensure ImGui context is properly shared across DLL boundary
		if (m_ImGuiLayer)
		{
			void* context = m_ImGuiLayer->GetImGuiContext();
			if (context)
			{
				// Force the current context to be set in both DLL and EXE
				ImGui::SetCurrentContext(static_cast<ImGuiContext*>(context));
				
				// Mark context as shared to prevent premature destruction
				m_ImGuiLayer->SetContextSharedWithDLL(true);
				HK_CORE_INFO("ImGui context shared with game DLL");
			}
		}
	}
}

