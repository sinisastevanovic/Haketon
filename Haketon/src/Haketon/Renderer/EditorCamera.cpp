#include "hkpch.h"
#include "EditorCamera.h"

#include "Haketon/Core/Input.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


namespace Haketon
{
    EditorCamera::EditorCamera(float FOV, float AspectRatio, float NearClip, float FarClip)
        : m_FOV(FOV), m_AspectRatio(AspectRatio), m_NearClip(NearClip), m_FarClip(FarClip), Camera(glm::perspective(glm::radians(FOV), AspectRatio, NearClip, FarClip))
    {
        UpdateView();
    }

    void EditorCamera::UpdateProjection()
    {
        m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
        m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
    }

    void EditorCamera::UpdateView()
    {
        //m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
        m_Position = CalculatePosition();

        glm::quat Orientation = GetOrientation();
        m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(Orientation);
        m_ViewMatrix = glm::inverse(m_ViewMatrix);
    }

    std::pair<float, float> EditorCamera::PanSpeed() const
    {
        float X = std::min(m_ViewportWidth / 1000.0f, 2.4f);
        float XFactor = 0.0366f * (X * X) - 0.1778f * X + 0.3021f;

        float Y = std::min(m_ViewportHeight / 1000.0f, 2.4f);
        float YFactor = 0.0366f * (Y * Y) - 0.1778f * Y + 0.3021f;

        return { XFactor, YFactor };
    }

    float EditorCamera::RotationSpeed() const
    {
        return 0.8f;
    }

    float EditorCamera::ZoomSpeed() const
    {
        float Distance = m_Distance * 0.2f;
        Distance = std::max(Distance, 0.0f);
        float Speed = Distance * Distance;
        Speed = std::min(Speed, 100.0f);
        return Speed;
    }

    void EditorCamera::OnUpdate(Timestep Ts)
    {
        const FVec2& Mouse { Input::GetMouseX(), Input::GetMouseY() };
        FVec2 Delta = (Mouse - m_InitialMousePosition) * 0.003f;
        m_InitialMousePosition = Mouse;
        
        if(Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
        {            
            if(Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift))
                MousePan(Delta);
            else if(Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl))
                MouseZoom(Delta.y);
            else
                MouseRotate(Delta);

            UpdateView();
        }

    }

    void EditorCamera::OnEvent(Event& E)
    {
        EventDispatcher Dispatcher(E);
        Dispatcher.Dispatch<MouseScrolledEvent>(HK_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
    }

    bool EditorCamera::OnMouseScroll(MouseScrolledEvent& E)
    {
        float Delta = E.GetYOffset() * 0.1f;
        MouseZoom(Delta);
        UpdateView();
        return false;
    }
  
    void EditorCamera::MousePan(const FVec2& Delta)
    {
        auto [XSpeed, YSpeed] = PanSpeed();
        m_FocalPoint += -GetRightDirection() * Delta.x * XSpeed * m_Distance;
        m_FocalPoint += GetUpDirection() * Delta.y * YSpeed * m_Distance;
    }

    void EditorCamera::MouseRotate(const FVec2& Delta)
    {
        float YawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
        m_Yaw += YawSign * Delta.x * RotationSpeed();
        m_Pitch += Delta.y * RotationSpeed();
    }

    void EditorCamera::MouseZoom(float Delta)
    {
        m_Distance -= Delta * ZoomSpeed();
        if(m_Distance < 1.0f)
        {
            m_FocalPoint += GetForwardDirection();
            m_Distance = 1.0f;
        }
    }

    glm::vec3 EditorCamera::GetUpDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 EditorCamera::GetRightDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 EditorCamera::GetForwardDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 EditorCamera::CalculatePosition() const
    {
        return m_FocalPoint - GetForwardDirection() * m_Distance;
    }

    glm::quat EditorCamera::GetOrientation() const
    {
        return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
    }
}
