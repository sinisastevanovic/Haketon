#pragma once

#include "Haketon/Core/Core.h"
#include "KeyCodes.h"
#include "MouseCodes.h"

#include "Haketon/Math/Math.h"

namespace Haketon {

	class HK_API Input
	{

	public:
		static bool IsKeyPressed(KeyCode key);
		
		static bool IsMouseButtonPressed(MouseCode button);
		static FVec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

		static void SetClipboardText(const std::string& text);
		static std::string GetClipboardText();
	};
}
