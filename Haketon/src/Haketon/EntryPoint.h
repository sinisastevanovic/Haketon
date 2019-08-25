#pragma once

#ifdef HK_PLATFORM_WINDOWS

extern Haketon::Application* Haketon::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Haketon::CreateApplication();
	app->Run();
	delete app;
}

#endif