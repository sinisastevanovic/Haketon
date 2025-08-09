#pragma once

#include "Haketon/Core/Core.h"
#include "Application.h"

namespace Haketon
{
	class HK_API EditorApplication : public Application
	{
	public:
		EditorApplication(const std::string& name = "Haketon Editor", ApplicationCommandLineArgs args = ApplicationCommandLineArgs(), bool maximized = true)
			: Application(ApplicationType::Editor, name, args, maximized)
		{
		}

		virtual ~EditorApplication() = default;
	};
}