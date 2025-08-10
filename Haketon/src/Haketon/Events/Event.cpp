#include "hkpch.h"
#include "Event.h"
#include "Haketon/Core/Application.h"

namespace Haketon {

	void Event::Dispatch(Event& event)
	{
		Application::Get().OnEvent(event);
	}

}