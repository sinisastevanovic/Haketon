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
		//HK_INFO("ExampleLayer::Update");
	}

	void OnEvent(Haketon::Event& event) override
	{
		//HK_TRACE("{0}", event);
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