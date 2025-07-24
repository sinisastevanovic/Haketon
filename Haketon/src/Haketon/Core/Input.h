#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"

#include "Haketon/Math/Math.h"

namespace Haketon {

	class Input
	{

	public:
		static bool IsKeyPressed(KeyCode key);
		
		static bool IsMouseButtonPressed(MouseCode button);
		static FVec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}
