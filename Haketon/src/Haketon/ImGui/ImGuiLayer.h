#pragma once

#include "Haketon/Core/Core.h"
#include "Haketon/Core/Layer.h"

#include "Haketon/Events/ApplicationEvent.h"
#include "Haketon/Events/KeyEvent.h"
#include "Haketon/Events/MouseEvent.h"

namespace Haketon {

	class HK_API ImGuiLayer : public Layer
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

		// DLL Context Management
		void* GetImGuiContext();
		void SetImGuiContext(void* context);
		void SetContextSharedWithDLL(bool shared) { m_ContextSharedWithDLL = shared; }
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
		bool m_ContextSharedWithDLL = false;
	};
}


