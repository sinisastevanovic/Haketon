#pragma once

#include "Core.h"
#include "ModuleManager.h"

#include "Window.h"
#include "Haketon/Core/LayerStack.h"
#include "Haketon/Events/Event.h"
#include "Haketon/Events/ApplicationEvent.h"

#include "Haketon/Core/Timestep.h"

#include "Haketon/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Haketon
{
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int Index) const
		{
			HK_CORE_ASSERT(Index < Count);
			return Args[Index];
		}
	};
	
	class Application
	{

	public:
		Application(const std::string& name = "Haketon App", ApplicationCommandLineArgs args = ApplicationCommandLineArgs());
		virtual ~Application();

		void OnEvent(Event& e);

		void PushLayer(Layer* Layer);
		void PushOverlay(Layer* Layer);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }

		ApplicationCommandLineArgs GetCommandLineArgs() const { return m_CommandLineArgs; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		Ref<ModuleManager> GetModuleManager() { return m_ModuleManager; }

	private:
		void Run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		ApplicationCommandLineArgs m_CommandLineArgs;
		
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		Ref<ModuleManager> m_ModuleManager;
		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);

	};

	// To be defined in a client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
