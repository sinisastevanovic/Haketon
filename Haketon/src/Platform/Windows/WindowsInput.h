#pragma once

#include "Haketon/Core/Input.h"

namespace Haketon {

	class WindowsInput : public Input
	{
	protected:
		virtual bool IsKeyPressedImpl(KeyCode key) override;
		virtual bool IsMouseButtonPressedImpl(MouseCode button) override;
		virtual glm::vec2 GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};
}