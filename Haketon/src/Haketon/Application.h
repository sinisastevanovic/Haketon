#pragma once

#include "Core.h"

namespace Haketon
{
	class __declspec(dllexport) Application
	{

	public:
		Application();
		virtual ~Application();

		void Run();

	};

	// To be defined in a client
	Application* CreateApplication();
}
