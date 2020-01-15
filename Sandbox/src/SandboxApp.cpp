#include <Haketon.h>

class ExampleLayer : public Haketon::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		/*if (Haketon::Input::IsKeyPressed(HK_KEY_TAB))
			HK_TRACE("Tab key is pressed!");*/
	}

	void OnEvent(Haketon::Event& event) override
	{
		// example
		/*if (event.GetEventType() == Haketon::EventType::KeyPressed)
		{
			Haketon::KeyPressedEvent& e = (Haketon::KeyPressedEvent&)event;
			if (e.GetKeyCode() == HK_KEY_SPACE)
				HK_TRACE("Tab key was pressed");
		}*/
	}
};

class Sandbox : public Haketon::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new Haketon::ImGuiLayer());
	}

	~Sandbox()
	{

	}
};

Haketon::Application* Haketon::CreateApplication()
{
	return new Sandbox();
}