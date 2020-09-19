#pragma once

#include "Core.h"

#include "Window.h"
#include "Haketon/LayerStack.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"

#include "Haketon/ImGui/ImGuiLayer.h"

#include "Haketon/Renderer/Shader.h"
#include "Haketon/Renderer/Buffer.h"
#include "Haketon/Renderer/VertexArray.h"


namespace Haketon
{
	class HAKETON_API Application
	{

	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* Layer);
		void PushOverlay(Layer* Layer);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;

		LayerStack m_LayerStack;

		static Application* s_Instance;

		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_VertexArray;

		std::shared_ptr<Shader> m_Shader2;
		std::shared_ptr<VertexArray> m_SquareVA;

	};

	// To be defined in a client
	Application* CreateApplication();
}
