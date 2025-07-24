#include "ExampleLayer.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

ExampleLayer::ExampleLayer()
	: Layer("Example"), m_CameraController(1280.0f / 720.0f, true)
{

}

void ExampleLayer::OnAttach()
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

void ExampleLayer::OnDetach()
{
}

void ExampleLayer::OnUpdate(Haketon::Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	Haketon::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Haketon::RenderCommand::Clear();

	Haketon::Renderer::BeginScene(m_CameraController.GetCamera());

	//Haketon::Renderer::Submit(m_TriangleShader, m_TriVertexArray);
	
	static glm::mat4 scale = glm::scale(glm::mat4(1.0f), FVec3(0.1f));

	// We have to bind the shader here, so it is available for uniform setting
	m_flatColorShader->Bind();
	for(int y = 0; y < 20; y++)
	{
		for(int x = 0; x < 20; x++)
		{
			FVec3 pos(x * 0.11f, y * 0.11f, 0.0f);
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
	Haketon::Renderer::Submit(textureShader, m_SquareVertexArray, glm::scale(glm::mat4(1.0f), FVec3(1.5f)));

	m_LogoTexture->Bind();
	Haketon::Renderer::Submit(textureShader, m_SquareVertexArray, glm::scale(glm::mat4(1.0f), FVec3(1.5f)));

	Haketon::Renderer::EndScene();

}

void ExampleLayer::OnEvent(Haketon::Event& e)
{
	m_CameraController.OnEvent(e);
}

void ExampleLayer::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::Text("Hello World");
	ImGui::ColorEdit3("Color1", glm::value_ptr(m_Color1));
	ImGui::ColorEdit3("Color2", glm::value_ptr(m_Color2));
	ImGui::End();
}