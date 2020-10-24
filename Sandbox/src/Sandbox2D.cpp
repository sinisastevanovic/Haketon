#include "Sandbox2D.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const uint32_t s_MapWidth = 24;
static const char* s_MapTiles =
	"000000000000000000000000"
	"000000001111111000000000"
	"000001111111111100000000"
	"000011111111111110000000"
	"000111111111111111000000"
	"001111111000111111100000"
	"001111110000011111110000"
	"001111111000111111110000"
	"000011111111111111110000"
	"000001111111111111100000"
	"000000111111111111000000"
	"000000000000000000000000";

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	HK_PROFILE_FUNCTION();

	m_Texture = Haketon::Texture2D::Create();
	m_SpriteSheet = Haketon::Texture2D::Create("assets/textures/RPGpack_sheet_2X.png", true);

	m_WaterSubTexture = Haketon::SubTexture2D::CreateFromCoords(m_SpriteSheet, {11.0f, 11.0f}, { 128.0f, 128.0f });

	m_MapWidth = s_MapWidth;
	m_MapHeight = strlen(s_MapTiles) / s_MapWidth;

	m_TextureMap['0'] = Haketon::SubTexture2D::CreateFromCoords(m_SpriteSheet, {11.0f, 11.0f}, { 128.0f, 128.0f });
	m_TextureMap['1'] = Haketon::SubTexture2D::CreateFromCoords(m_SpriteSheet, {6.0f, 11.0f}, { 128.0f, 128.0f });
	

    m_Particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
	m_Particle.LifeTime = 1.0f;
	m_Particle.Velocity = { 0.0f, 0.0f };
	m_Particle.VelocityVariation = { 3.0f, 1.0f };
	m_Particle.Position = { 0.0f, 0.0f };

	m_CameraController.SetZoomLevel(5.0f);
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
	uint32_t maxQuads = 10;
	for(uint32_t x = 0; x < maxQuads; x++)
	{
		for(uint32_t y = 0; y < maxQuads; y++)
		{
			Haketon::Renderer2D::DrawRotatedQuad({x, y, -0.1f}, glm::radians(45.0f),{1.0f, 1.0f}, {(float)x / (float)maxQuads, (float)y / (float)maxQuads, 0.5f, 1.0f});
		}
	}

	Haketon::Renderer2D::DrawQuad(m_WaterSubTexture, { 0, 0, 0});

	for(uint32_t y = 0; y < m_MapHeight; y++)
	{
		for(uint32_t x = 0; x < m_MapWidth; x++)
		{
			char tileType = s_MapTiles[x + y * m_MapWidth];
			if(m_TextureMap.find(tileType) != m_TextureMap.end())
			{
				Haketon::Ref<Haketon::SubTexture2D> texture = m_TextureMap[tileType];
				Haketon::Renderer2D::DrawQuad(m_TextureMap[tileType], { x, m_MapHeight - y - 1.0f, 0 });
			}
		}
	}

	Haketon::Renderer2D::EndScene();


	/*if (Haketon::Input::IsMouseButtonPressed(Haketon::Mouse::ButtonLeft))
	{
		auto mousePos = Haketon::Input::GetMousePosition();
		auto width = Haketon::Application::Get().GetWindow().GetWidth();
		auto height = Haketon::Application::Get().GetWindow().GetHeight();

		auto bounds = m_CameraController.GetBounds();
		auto pos = m_CameraController.GetCamera().GetPosition();
		mousePos.x = (mousePos.x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
		mousePos.y = bounds.GetHeight() * 0.5f - (mousePos.y / height) * bounds.GetHeight();
		m_Particle.Position = { mousePos.x + pos.x, mousePos.y + pos.y };
		for (int i = 0; i < 5; i++)
			m_ParticleSystem.Emit(m_Particle);
	}

	m_ParticleSystem.OnUpdate(ts);
	m_ParticleSystem.OnRender(m_CameraController.GetCamera());*/

	
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
	ImGui::SliderAngle("Rotation", &TexQuadRotation);
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
