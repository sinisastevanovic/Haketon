#pragma once

namespace Haketon {

	class HK_API GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;
			
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
	};
}