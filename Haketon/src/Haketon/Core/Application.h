#pragma once

#include "Core.h"

#include "Window.h"
#include "Haketon/Core/LayerStack.h"
#include "Haketon/Events/Event.h"
#include "Haketon/Events/ApplicationEvent.h"

#include "Haketon/Core/Timestep.h"

#include "Haketon/ImGui/ImGuiLayer.h"

namespace Haketon
{
	class Application
	{

	public:
		Application();
		virtual ~Application() = default;

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* Layer);
		void PushOverlay(Layer* Layer);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;

	};

	// To be defined in a client
	Application* CreateApplication();
}
