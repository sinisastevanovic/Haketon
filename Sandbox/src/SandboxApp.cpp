#include "Haketon.h"

class Sandbox : public Haketon::Application
{
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}
};

Haketon::Application* Haketon::CreateApplication()
{
	return new Sandbox();
}