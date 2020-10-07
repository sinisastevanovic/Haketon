#include <Haketon.h>

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>

#include "glm/gtc/type_ptr.hpp"

#include "Haketon/Renderer/Shader.h"

class ExampleLayer : public Haketon::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f)
	{
		m_TriVertexArray = Haketon::VertexArray::Create();

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f
		};

		Haketon::Ref<Haketon::VertexBuffer> triangleVB = Haketon::VertexBuffer::Create(vertices, sizeof(vertices));
		Haketon::BufferLayout layout = {
			{ Haketon::ShaderDataType::Float3, "a_Position" },
			{ Haketon::ShaderDataType::Float4, "a_Color" }
		};
		triangleVB->SetLayout(layout);
		m_TriVertexArray->AddVertexBuffer(triangleVB);

		uint32_t indices[3] = { 0, 1, 2 };
		Haketon::Ref<Haketon::IndexBuffer> triangleIB = Haketon::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		m_TriVertexArray->SetIndexBuffer(triangleIB);

		m_SquareVertexArray = Haketon::VertexArray::Create();

		float squareVerts[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Haketon::Ref<Haketon::VertexBuffer> squareVB = Haketon::VertexBuffer::Create(squareVerts, sizeof(squareVerts));
		squareVB->SetLayout({
			{ Haketon::ShaderDataType::Float3, "a_Position" },
			{ Haketon::ShaderDataType::Float2, "a_TexCoord" }
		});
		m_SquareVertexArray->AddVertexBuffer(squareVB);

		uint32_t indices2[6] = { 0, 1, 2, 2, 3, 0 };
		Haketon::Ref<Haketon::IndexBuffer> squareIB = Haketon::IndexBuffer::Create(indices2, sizeof(indices2) / sizeof(uint32_t));
		m_SquareVertexArray->SetIndexBuffer(squareIB);

		std::string triangleVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
		
			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
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

		m_TriangleShader = Haketon::Shader::Create("VertexPosColor", triangleVertexSrc, triangleFragmentSrc);

		std::string flatColorVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
	
			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string flatColorFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0);
			}
		)";

		m_flatColorShader = Haketon::Shader::Create("FlatColor", flatColorVertexSrc, flatColorFragmentSrc);

		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		m_Texture = Haketon::Texture2D::Create();
		m_LogoTexture = Haketon::Texture2D::Create("assets/textures/ChernoLogo.png");

		textureShader->Bind();
		textureShader->SetInt("u_Texture", 0);
	}

	void OnUpdate(Haketon::Timestep ts) override
	{		
		if (Haketon::Input::IsKeyPressed(HK_KEY_W))
			m_CameraPosition.y += m_CameraSpeed * ts;
		if(Haketon::Input::IsKeyPressed(HK_KEY_A))
			m_CameraPosition.x -= m_CameraSpeed * ts;
		if(Haketon::Input::IsKeyPressed(HK_KEY_S))
			m_CameraPosition.y -= m_CameraSpeed * ts;
		if(Haketon::Input::IsKeyPressed(HK_KEY_D))
			m_CameraPosition.x += m_CameraSpeed * ts;
		if(Haketon::Input::IsKeyPressed(HK_KEY_Q))
			m_CameraRotation -= m_CameraRotSpeed * ts;
		if(Haketon::Input::IsKeyPressed(HK_KEY_E))
			m_CameraRotation += m_CameraRotSpeed * ts;

		Haketon::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Haketon::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Haketon::Renderer::BeginScene(m_Camera);

		//Haketon::Renderer::Submit(m_TriangleShader, m_TriVertexArray);
		
		static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		for(int y = 0; y < 20; y++)
		{
			for(int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				if(x % 2 == 0)
					m_flatColorShader->SetFloat3("u_Color", m_Color1);
				else
					m_flatColorShader->SetFloat3("u_Color", m_Color2);
				Haketon::Renderer::Submit(m_flatColorShader, m_SquareVertexArray, transform);
			}		
		}

		auto textureShader = m_ShaderLibrary.Get("Texture");

		m_Texture->Bind();
		Haketon::Renderer::Submit(textureShader, m_SquareVertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		m_LogoTexture->Bind();
		Haketon::Renderer::Submit(textureShader, m_SquareVertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

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
		ImGui::Begin("Settings");
		ImGui::Text("Hello World");
		ImGui::ColorEdit3("Color1", glm::value_ptr(m_Color1));
		ImGui::ColorEdit3("Color2", glm::value_ptr(m_Color2));
		ImGui::End();
	}

private:

	Haketon::ShaderLibrary m_ShaderLibrary;
	Haketon::Ref<Haketon::Shader> m_TriangleShader;
	Haketon::Ref<Haketon::VertexArray> m_TriVertexArray;

	Haketon::Ref<Haketon::Shader> m_flatColorShader;
	Haketon::Ref<Haketon::VertexArray> m_SquareVertexArray;

	Haketon::Ref<Haketon::Texture2D> m_Texture;
	Haketon::Ref<Haketon::Texture2D> m_LogoTexture;

	Haketon::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraRotation = 0.0f;
	float m_CameraSpeed = 1.0f;
	float m_CameraRotSpeed = 10.0f;

	glm::vec3 m_Color1 = { 0.8f, 0.2f, 0.3f };
	glm::vec3 m_Color2 = { 0.2f, 0.3f, 0.8f };
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