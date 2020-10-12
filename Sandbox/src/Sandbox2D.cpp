#include "Sandbox2D.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

template<typename Fn>
class Timer
{
public:
	Timer(const char* name, Fn&& func)
		: m_Name(name), m_Func(func), m_Stopped(false)
	{
		m_StartTimepoint = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		if(!m_Stopped)
			Stop();
	}
	
	void Stop()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		m_Stopped = true;

		float duration = (end - start) * 0.001f;
		m_Func({m_Name, duration});		
	}

private:
	const char* m_Name;
	std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
	bool m_Stopped;
	Fn m_Func;
};

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); })

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	m_Texture = Haketon::Texture2D::Create();
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(Haketon::Timestep ts)
{
	Timer timer1("Sandbox2D::OnUpdate", [&](ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); });

	{
		PROFILE_SCOPE("CameraController::OnUpdate");
		m_CameraController.OnUpdate(ts);
	}

	{
		PROFILE_SCOPE("Renderer Prep");
		Haketon::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Haketon::RenderCommand::Clear();
	}

	{
		PROFILE_SCOPE("Renderer Draw");
		Haketon::Renderer2D::BeginScene(m_CameraController.GetCamera());

		Haketon::Renderer2D::DrawQuad({0.5f, -0.5f, 0.0f}, 0.0f, {0.5f, 0.75f}, {0.2f, 0.3f, 0.8f, 1.0f});
		Haketon::Renderer2D::DrawQuad(TexQuadPosition, TexQuadRotation, TexQuadScale, m_Texture, m_TextureTint);

		Haketon::Renderer2D::EndScene();
	}

	timer1.Stop();
	
	m_TotalOnUpdateTime += m_ProfileResults[m_ProfileResults.size() - 1].Time;
	m_NumUpdates++;
}

void Sandbox2D::OnEvent(Haketon::Event& e)
{
    m_CameraController.OnEvent(e);
}

void Sandbox2D::OnImGuiRender()
{
    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Tint", glm::value_ptr(m_TextureTint));
	ImGui::SliderFloat3("Position", glm::value_ptr(TexQuadPosition), -0.5f, 0.5f);
	ImGui::SliderAngle("Rotation", &TexQuadRotation);
	ImGui::SliderFloat2("Scale", glm::value_ptr(TexQuadScale), -1.0f, 1.0f);
	ImGui::End();

	ImGui::Begin("Profiling");
	for(auto& result : m_ProfileResults)
	{
		char label[50];
		strcpy(label, "%.3fms ");
		strcat(label, result.Name);
		ImGui::Text(label, result.Time);
	}

	char label[50];
	strcpy(label, "%.3fms ");
	strcat(label, "Average");
	ImGui::Text(label, m_TotalOnUpdateTime / (float)m_NumUpdates);
	m_ProfileResults.clear();
	ImGui::End();
	
}
