#pragma once

#ifdef HK_PLATFORM_WINDOWS

extern Haketon::Application* Haketon::CreateApplication();

int main(int argc, char** argv)
{
	Haketon::Log::Init();
	HK_CORE_WARN("Initialized Log!");
	int a = 5;
	HK_INFO("Hello! Var={0}", a);

	auto app = Haketon::CreateApplication();
	app->Run();
	delete app;
}

#endif