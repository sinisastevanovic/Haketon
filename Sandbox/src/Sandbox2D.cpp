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
	HK_PROFILE_FUNCTION();

	m_Texture = Haketon::Texture2D::Create();
}

void Sandbox2D::OnDetach()
{
	HK_PROFILE_FUNCTION();

}

void Sandbox2D::OnUpdate(Haketon::Timestep ts)
{
	HK_PROFILE_FUNCTION();

	m_CameraController.OnUpdate(ts);

	Haketon::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Haketon::RenderCommand::Clear();
	
	Haketon::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Haketon::Renderer2D::DrawQuad({0.5f, -0.5f, 0.0f}, {0.5f, 0.75f}, {0.2f, 0.3f, 0.8f, 1.0f});
	Haketon::Renderer2D::DrawRotatedQuad(TexQuadPosition, TexQuadRotation, TexQuadScale, m_Texture, m_TextureTint, 1.0f);

	Haketon::Renderer2D::EndScene();
}

void Sandbox2D::OnEvent(Haketon::Event& e)
{
    m_CameraController.OnEvent(e);
}

void Sandbox2D::OnImGuiRender()
{
	HK_PROFILE_FUNCTION();

    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Tint", glm::value_ptr(m_TextureTint));
	ImGui::SliderFloat3("Position", glm::value_ptr(TexQuadPosition), -0.5f, 0.5f);
	ImGui::SliderAngle("Rotation", &TexQuadRotation);
	ImGui::SliderFloat2("Scale", glm::value_ptr(TexQuadScale), -1.0f, 1.0f);
	ImGui::End();
}
