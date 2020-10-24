#include <Haketon.h>
#include <Haketon/Core/EntryPoint.h>

#include "Sandbox2D.h"
#include "ExampleLayer.h"

class HaketonEditor : public Haketon::Application
{
public:
	HaketonEditor()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~HaketonEditor()
	{

	}
};

Haketon::Application* Haketon::CreateApplication()
{
	return new HaketonEditor();
}