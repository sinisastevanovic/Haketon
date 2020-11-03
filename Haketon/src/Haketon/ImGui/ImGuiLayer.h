#pragma once

#include "Haketon/Core/Layer.h"

#include "Haketon/Events/ApplicationEvent.h"
#include "Haketon/Events/KeyEvent.h"
#include "Haketon/Events/MouseEvent.h"

namespace Haketon {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		virtual void OnEvent(Event& event) override;
		

		void Begin();
		void End();

		void SetBlockEvents(bool block) { m_BlockEvents = block; }

		void SetDarkThemeColors();
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};
}


