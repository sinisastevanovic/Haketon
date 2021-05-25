#pragma once

#include "Haketon/Core/Application.h"

#ifdef HK_PLATFORM_WINDOWS

extern Haketon::Application* Haketon::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	Haketon::Log::Init();
	
	HK_PROFILE_BEGIN_SESSION("Startup", "HaketonProfile-Startup.json");
	auto app = Haketon::CreateApplication({ argc, argv });
	HK_PROFILE_END_SESSION();
	
	HK_PROFILE_BEGIN_SESSION("Runtime", "HaketonProfile-Runtime.json");
	app->Run();
	HK_PROFILE_END_SESSION();
	
	HK_PROFILE_BEGIN_SESSION("Shutdown", "HaketonProfile-Shutdown.json");
	delete app;
	HK_PROFILE_END_SESSION();
}

#endif