#pragma once

#include "Haketon/Renderer/Camera.h"
#include <rttr/type>
namespace Haketon
{
    ENUM()
    enum class ProjectionType
    {
        Perspective = 0,
        Orthographic = 1
    };
    
    STRUCT()
    class SceneCamera : public Camera
    {           
    public:
        SceneCamera();
        virtual ~SceneCamera() = default;


        void SetViewportSize(uint32_t width, uint32_t height);

        void SetPerspective(float verticalFOV = glm::radians(45.0f), float nearClip = 0.01f, float farClip = 10000.0f);
        float GetPerspectiveVerticalFOV() const { return m_PerspectiveFOV; }
        void SetPerspectiveVerticalFOV(float fov) { m_PerspectiveFOV = fov; RecalculateProjection(); }
        float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
        void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; RecalculateProjection(); }       
        float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
        void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; RecalculateProjection(); }

        void SetOrthographic(float size = 10.0f, float nearClip = -1.0f, float farClip = 1.0f);
        float GetOrthographicSize() const { return m_OrthographicSize; }
        void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
        float GetOrthographicNearClip() const { return m_OrthographicNear; }
        void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }       
        float GetOrthographicFarClip() const { return m_OrthographicFar; }
        void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }

        ProjectionType GetProjectionType() const { return m_ProjectionType; }
        void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }


    private:
        void RecalculateProjection();
        
    private:
        PROPERTY(Getter=GetProjectionType, Setter=SetProjectionType)
        ProjectionType m_ProjectionType = ProjectionType::Orthographic;

        PROPERTY(Getter=GetPerspectiveVerticalFOV, Setter=SetPerspectiveVerticalFOV, EditCondition=m_ProjectionType==Perspective)
        float m_PerspectiveFOV = glm::radians(45.0f);

        PROPERTY(Getter=GetPerspectiveNearClip, Setter=SetPerspectiveNearClip, EditCondition=m_ProjectionType==Perspective)
        float m_PerspectiveNear = 0.01f;

        PROPERTY(Getter=GetPerspectiveFarClip, Setter=SetPerspectiveFarClip, EditCondition=m_ProjectionType==Perspective)
        float m_PerspectiveFar = 1000.0f;

        PROPERTY(Getter=GetOrthographicSize, Setter=SetOrthographicSize, EditCondition=m_ProjectionType==Orthographic)
        float m_OrthographicSize = 10.0f;

        PROPERTY(Getter=GetOrthographicNearClip, Setter=SetOrthographicNearClip, EditCondition=m_ProjectionType==Orthographic)
        float m_OrthographicNear = -1.0f;

        PROPERTY(Getter=GetOrthographicFarClip, Setter=SetOrthographicFarClip, EditCondition=m_ProjectionType==Orthographic)
        float m_OrthographicFar = 1.0f;

        float m_AspectRatio = 1.0f;

        RTTR_ENABLE();
    };

}
