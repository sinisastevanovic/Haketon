#include <Haketon.h>

#include "imgui/imgui.h"

class ExampleLayer : public Haketon::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f)
	{
		m_TriVertexArray.reset(Haketon::VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f
		};

		std::shared_ptr<Haketon::VertexBuffer> triangleVB;
		triangleVB.reset(Haketon::VertexBuffer::Create(vertices, sizeof(vertices)));
		Haketon::BufferLayout layout = {
			{ Haketon::ShaderDataType::Float3, "a_Position" },
			{ Haketon::ShaderDataType::Float4, "a_Color" }
		};
		triangleVB->SetLayout(layout);
		m_TriVertexArray->AddVertexBuffer(triangleVB);

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<Haketon::IndexBuffer> triangleIB;
		triangleIB.reset(Haketon::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_TriVertexArray->SetIndexBuffer(triangleIB);

		m_SquareVertexArray.reset(Haketon::VertexArray::Create());

		float squareVerts[3 * 4] = {
			-0.75f, -0.75f, 0.0f,
			 0.75f, -0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			 -0.75f,  0.75f, 0.0f,
		};

		std::shared_ptr<Haketon::VertexBuffer> squareVB;
		squareVB.reset(Haketon::VertexBuffer::Create(squareVerts, sizeof(squareVerts)));
		squareVB->SetLayout({ { Haketon::ShaderDataType::Float3, "a_Position" } });
		m_SquareVertexArray->AddVertexBuffer(squareVB);

		uint32_t indices2[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<Haketon::IndexBuffer> squareIB;
		squareIB.reset(Haketon::IndexBuffer::Create(indices2, sizeof(indices2) / sizeof(uint32_t)));
		m_SquareVertexArray->SetIndexBuffer(squareIB);

		std::string triangleVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
		
			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}
		)";

		std::string triangleFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		)";

		m_TriangleShader.reset(Haketon::Shader::Create(triangleVertexSrc, triangleFragmentSrc));

		std::string squareVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
	
			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}
		)";

		std::string squareFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			void main()
			{
				color = vec4(0.2, 0.3, 0.8, 1.0f);
			}
		)";

		m_BlueShader.reset(Haketon::Shader::Create(squareVertexSrc, squareFragmentSrc));
	}

	void OnUpdate() override
	{
		if (Haketon::Input::IsKeyPressed(HK_KEY_W))
			m_CameraPosition.y -= m_CameraSpeed;
		if(Haketon::Input::IsKeyPressed(HK_KEY_S))
			m_CameraPosition.y += m_CameraSpeed;
		if(Haketon::Input::IsKeyPressed(HK_KEY_A))
			m_CameraPosition.x += m_CameraSpeed;
		if(Haketon::Input::IsKeyPressed(HK_KEY_D))
			m_CameraPosition.x -= m_CameraSpeed;
		if(Haketon::Input::IsKeyPressed(HK_KEY_Q))
			m_CameraRotation -= m_CameraRotSpeed;
		if(Haketon::Input::IsKeyPressed(HK_KEY_E))
			m_CameraRotation += m_CameraRotSpeed;

		Haketon::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Haketon::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Haketon::Renderer::BeginScene(m_Camera);

		Haketon::Renderer::Submit(m_BlueShader, m_SquareVertexArray);
		Haketon::Renderer::Submit(m_TriangleShader, m_TriVertexArray);

		Haketon::Renderer::EndScene();

	}

	void OnEvent(Haketon::Event& event) override
	{
		Haketon::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Haketon::KeyPressedEvent>(HK_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
	}

	bool OnKeyPressedEvent(Haketon::KeyPressedEvent& event)
	{
		/*if (event.GetKeyCode() == HK_KEY_W)
			m_CameraPosition.y -= m_CameraSpeed;
		else if(event.GetKeyCode() == HK_KEY_S)
			m_CameraPosition.y += m_CameraSpeed;
		else if(event.GetKeyCode() == HK_KEY_A)
			m_CameraPosition.x += m_CameraSpeed;
		else if(event.GetKeyCode() == HK_KEY_D)
			m_CameraPosition.x -= m_CameraSpeed;
		else if(event.GetKeyCode() == HK_KEY_Q)
			m_Camera.SetRotation(m_Camera.GetRotation() - 5.0f);
		else if(event.GetKeyCode() == HK_KEY_E)
			m_Camera.SetRotation(m_Camera.GetRotation() + 5.0f);*/


		return false;
	}

	void OnImGuiRender() override
	{
		/*ImGui::Begin("Test");
		ImGui::Text("Hello World");
		ImGui::End();*/
	}

private:

	std::shared_ptr<Haketon::Shader> m_TriangleShader;
	std::shared_ptr<Haketon::VertexArray> m_TriVertexArray;

	std::shared_ptr<Haketon::Shader> m_BlueShader;
	std::shared_ptr<Haketon::VertexArray> m_SquareVertexArray;

	Haketon::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraRotation = 0.0f;
	float m_CameraSpeed = 0.01f;
	float m_CameraRotSpeed = 0.5;
};

class Sandbox : public Haketon::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		//PushOverlay(new Haketon::ImGuiLayer());
	}

	~Sandbox()
	{

	}
};

Haketon::Application* Haketon::CreateApplication()
{
	return new Sandbox();
}