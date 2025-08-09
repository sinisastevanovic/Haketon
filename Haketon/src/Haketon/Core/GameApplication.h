#pragma once

#include "Application.h"

namespace Haketon
{
	class HK_API GameApplication : public Application
	{
	public:
		GameApplication(const std::string& name = "Haketon Game", ApplicationCommandLineArgs args = ApplicationCommandLineArgs(), bool maximized = false)
			: Application(ApplicationType::Game, name, args, maximized)
		{
		}

		virtual ~GameApplication() = default;
	};
}