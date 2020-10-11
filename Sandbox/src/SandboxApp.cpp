#include <Haketon.h>
#include <Haketon/Core/EntryPoint.h>

#include "Sandbox2D.h"
#include "ExampleLayer.h"

class Sandbox : public Haketon::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox()
	{

	}
};

Haketon::Application* Haketon::CreateApplication()
{
	return new Sandbox();
}