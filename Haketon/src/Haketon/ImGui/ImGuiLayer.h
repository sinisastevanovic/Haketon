#pragma once

#include "Haketon/Layer.h"

#include "Haketon/Events/ApplicationEvent.h"
#include "Haketon/Events/KeyEvent.h"
#include "Haketon/Events/MouseEvent.h"

namespace Haketon {

	class HAKETON_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
		

		void Begin();
		void End();

	private:
		float m_Time = 0.0f;
	};
}


