#include "Application.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"

namespace Haketon
{
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		HK_TRACE(e);

		while (true);
	}
}

