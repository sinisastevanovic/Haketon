#pragma once

#include "Haketon/Core/Core.h"
#include "Haketon/Events/Event.h"
#include "Haketon/Core/Timestep.h"

namespace Haketon {

	class Application;

	class HK_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }
		inline Application& GetApplication() const { return *m_Application; }

		void SetApplication(Application* app) { m_Application = app; }

	protected:
		std::string m_DebugName;
		Application* m_Application = nullptr;
	};
}

