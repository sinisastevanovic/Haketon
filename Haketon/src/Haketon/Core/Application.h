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
	class Scene;

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
	
	class HK_API Application
	{

	public:
		Application(const std::string& name = "Haketon App", ApplicationCommandLineArgs args = ApplicationCommandLineArgs(), bool maximized = false);
		virtual ~Application();

		virtual void OnEvent(Event& e);
		void Close();
		virtual void Shutdown();

		void PushLayer(Layer* Layer);
		void PushOverlay(Layer* Layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow();
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		ApplicationCommandLineArgs GetCommandLineArgs() const { return m_CommandLineArgs; }

		Scene* GetActiveScene() { return m_ActiveScene.get(); }
		
		void SetWindowTitle(const std::string& title);

		// DLL ImGui Context Management
		void ShareImGuiContext();

	protected:
		virtual void RunImpl() {}
		
	private:
		void Run();
		
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	protected:
		Ref<Scene> m_ActiveScene;
		
	private:
		ApplicationCommandLineArgs m_CommandLineArgs;
		
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in a client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
