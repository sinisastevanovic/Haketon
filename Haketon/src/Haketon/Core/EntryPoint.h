#pragma once

#include "Haketon/Core/Core.h"
#include "Haketon/Core/Application.h"
#include "Haketon/Core/ModuleManager.h"

#ifdef HK_PLATFORM_WINDOWS

extern Haketon::Application* Haketon::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	Haketon::Log::Init();

	// Initialize engine systems
	ModuleManager::Initialize();
	
	HK_PROFILE_BEGIN_SESSION("Startup", "HaketonProfile-Startup.json");
	auto app = Haketon::CreateApplication({ argc, argv });
	HK_PROFILE_END_SESSION();
	
	HK_PROFILE_BEGIN_SESSION("Runtime", "HaketonProfile-Runtime.json");
	app->Run();
	HK_PROFILE_END_SESSION();
	
	HK_PROFILE_BEGIN_SESSION("Shutdown", "HaketonProfile-Shutdown.json");
	app->Shutdown();
	delete app;
	ModuleManager::Shutdown();
	HK_PROFILE_END_SESSION();
}

#endif