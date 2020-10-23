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

	Haketon::Renderer2D::ResetStats();

	Haketon::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Haketon::RenderCommand::Clear();

	Haketon::Renderer2D::BeginScene(m_CameraController.GetCamera());
	uint32_t maxQuads = 320;
	for(uint32_t x = 0; x < maxQuads; x++)
	{
		for(uint32_t y = 0; y < maxQuads; y++)
		{
			Haketon::Renderer2D::DrawQuad({x, y, 0.0f}, {1.0f, 1.0f}, m_Texture, {(float)x / (float)maxQuads, (float)y / (float)maxQuads, 0.5f, 1.0f});
		}
	}
	//Haketon::Renderer2D::DrawRotatedQuad({0.0f, 0.0f, 0.0f}, 45.0f, {1.0f, 1.0f}, m_Texture, m_TextureTint, 1.0f);
	//Haketon::Renderer2D::DrawRotatedQuad({1.0f, 0.0f, 0.0f}, 55.0f, {1.0f, 1.0f}, m_Texture, m_TextureTint, 1.0f);
	//Haketon::Renderer2D::DrawRotatedQuad({0.0f, 1.0f, 0.0f}, -12.0f, {1.0f, 1.0f}, m_TextureTint);
	//Haketon::Renderer2D::DrawRotatedQuad({0.0f, -0.5f, 0.0f}, TexQuadRotation, {1.0f, 1.0f}, m_TextureTint);
	
	//Haketon::Renderer2D::DrawQuad({0.5f, -0.5f, 0.0f}, {0.5f, 0.75f}, {0.2f, 0.3f, 0.8f, 1.0f});
	//Haketon::Renderer2D::DrawQuad({-1.0f, 0.0f, 0.0f}, {0.8f, 0.8f}, {0.8f, 0.2f, 0.3f, 1.0f});
	//Haketon::Renderer2D::DrawRotatedQuad(TexQuadPosition, TexQuadRotation, TexQuadScale, m_Texture, m_TextureTint, 1.0f);
	//Haketon::Renderer2D::DrawQuad(TexQuadPosition, TexQuadScale, m_Texture, m_TextureTint, 10.0f);
	//Haketon::Renderer2D::DrawQuad({0.0f, 0.0f, 0.5f}, {0.2f, 0.2f}, m_Texture, m_TextureTint, 1.0f);

	Haketon::Renderer2D::EndScene();

	Haketon::Renderer2D::BeginScene(m_CameraController.GetCamera());
	
	Haketon::Renderer2D::DrawRotatedQuad(TexQuadPosition, TexQuadRotation, TexQuadScale, m_Texture, m_TextureTint, TexQuadTiling);
	
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
	ImGui::SliderFloat3("Position", glm::value_ptr(TexQuadPosition), -100.0f, 100.0f);
	ImGui::SliderFloat("Rotation", &TexQuadRotation, -360.0f, 360.0f);
	ImGui::SliderFloat2("Scale", glm::value_ptr(TexQuadScale), -100.0f, 100.0f);
	ImGui::SliderFloat("Tiling", &TexQuadTiling, 0.1f, 100.0f);
	ImGui::End();

	ImGui::Begin("Stats");
	auto stats = Haketon::Renderer2D::GetStats();
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quad Count: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
	ImGui::End();
}
