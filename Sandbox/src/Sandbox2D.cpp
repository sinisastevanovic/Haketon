#include "Sandbox2D.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
    m_SquareVA = Haketon::VertexArray::Create();

	float squareVerts[3 * 4] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f
	};

	Haketon::Ref<Haketon::VertexBuffer> squareVB = Haketon::VertexBuffer::Create(squareVerts, sizeof(squareVerts));
	squareVB->SetLayout({
		{ Haketon::ShaderDataType::Float3, "a_Position" }
	});
	m_SquareVA->AddVertexBuffer(squareVB);

	uint32_t indices2[6] = { 0, 1, 2, 2, 3, 0 };
	Haketon::Ref<Haketon::IndexBuffer> squareIB = Haketon::IndexBuffer::Create(indices2, sizeof(indices2) / sizeof(uint32_t));
	m_SquareVA->SetIndexBuffer(squareIB);

	m_FlatColorShader = Haketon::Shader::Create("assets/shaders/FlatColor.glsl");
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(Haketon::Timestep ts)
{
    m_CameraController.OnUpdate(ts);

	Haketon::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Haketon::RenderCommand::Clear();

	Haketon::Renderer::BeginScene(m_CameraController.GetCamera());

	m_FlatColorShader->Bind();
	m_FlatColorShader->SetFloat4("u_Color", m_Color);
	Haketon::Renderer::Submit(m_FlatColorShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

	Haketon::Renderer::EndScene();
}

void Sandbox2D::OnEvent(Haketon::Event& e)
{
    m_CameraController.OnEvent(e);
}

void Sandbox2D::OnImGuiRender()
{
    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Color", glm::value_ptr(m_Color));
    ImGui::End();
}
