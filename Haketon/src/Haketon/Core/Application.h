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
	enum class ApplicationType
	{
		Editor,
		Game
	};

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
		Application(ApplicationType type, const std::string& name = "Haketon App", ApplicationCommandLineArgs args = ApplicationCommandLineArgs(), bool maximized = false);
		virtual ~Application();

		void OnEvent(Event& e);

		void PushLayer(Layer* Layer);
		void PushOverlay(Layer* Layer);

		inline static Application& Get() { return *s_Instance; }
		inline static Application* GetEditor() { return s_EditorInstance; }
		inline static Application* GetGame() { return s_GameInstance; }
		inline ApplicationType GetApplicationType() const { return m_ApplicationType; }
		inline Window& GetWindow() { return *m_Window; }
		void SetWindowTitle(const std::string& title);

		ApplicationCommandLineArgs GetCommandLineArgs() const { return m_CommandLineArgs; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		// DLL ImGui Context Management
		void ShareImGuiContext();


		void UpdateLayers(Timestep timestep);

	private:
		void Run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		ApplicationType m_ApplicationType;
		ApplicationCommandLineArgs m_CommandLineArgs;
		
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
		static Application* s_EditorInstance;
		static Application* s_GameInstance;
		friend int ::main(int argc, char** argv);

	};

	// To be defined in a client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
