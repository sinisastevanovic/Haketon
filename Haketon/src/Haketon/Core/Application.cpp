#include "hkpch.h"

#include "Application.h"

#include "imgui.h"
#include "Haketon/Events/Event.h"
#include "Log.h"
#include "Reflection.h"

#include "Haketon/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

namespace Haketon
{

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name, ApplicationCommandLineArgs args, bool maximized)
		: m_CommandLineArgs(args)
	{
		HK_PROFILE_FUNCTION();
		
		HK_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// Initialize reflection system
		Reflection::Initialize();

		m_Window = Window::Create(WindowProps(name, maximized));
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		m_ModuleManager = CreateRef<ModuleManager>();
	}

	Application::~Application()
	{
		HK_PROFILE_FUNCTION();

		Renderer::Shutdown();
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

		m_LayerStack.PushLayer(Layer);
		Layer->OnAttach();
	}

	void Application::PushOverlay(Layer* Layer)
	{
		HK_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(Layer);
		Layer->OnAttach();
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
		}
	}

	void Application::Close()
	{
		m_Running = false;
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
}

