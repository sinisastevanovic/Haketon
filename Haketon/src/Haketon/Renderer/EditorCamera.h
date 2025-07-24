#pragma once

#include "Camera.h"
#include "Haketon/Core/Timestep.h"
#include "Haketon/Events/Event.h"
#include "Haketon/Events/MouseEvent.h"

#include <glm/glm.hpp>

namespace Haketon
{
    class EditorCamera : public Camera
    {
    public:
        EditorCamera() = default;
        EditorCamera(float FOV, float AspectRatio, float NearClip, float FarClip);

        void OnUpdate(Timestep Ts);
        void OnEvent(Event& E);

        inline float GetDistance() const { return m_Distance; }
        inline void SetDistance(float Distance) { m_Distance = Distance; }

        inline void SetViewportSize(float Width, float Height) { m_ViewportWidth = Width; m_ViewportHeight = Height; UpdateProjection(); }

        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

        glm::vec3 GetUpDirection() const;
        glm::vec3 GetRightDirection() const;
        glm::vec3 GetForwardDirection() const;
        const glm::vec3& GetPosition() const { return m_Position; }
        glm::quat GetOrientation() const;

        float GetPitch() const { return m_Pitch; }
        float GetYaw() const { return m_Yaw; }

    private:
        void UpdateProjection();
        void UpdateView();

        bool OnMouseScroll(MouseScrolledEvent& E);

        void MousePan(const FVec2& Delta);
        void MouseRotate(const FVec2& Delta);
        void MouseZoom(float Delta);

        glm::vec3 CalculatePosition() const;

        std::pair<float, float> PanSpeed() const;
        float RotationSpeed() const;
        float ZoomSpeed() const;

    private:
        float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

        glm::mat4 m_ViewMatrix;
        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

        FVec2 m_InitialMousePosition;

        float m_Distance = 10.0f;
        float m_Pitch = 0.0f, m_Yaw = 0.0f;

        float m_ViewportWidth = 1280, m_ViewportHeight = 720;
    
    };
}

