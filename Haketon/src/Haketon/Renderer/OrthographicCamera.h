﻿#pragma once

#include <glm/glm.hpp>

namespace Haketon
{  
    class OrthographicCamera
    {
    public:
        OrthographicCamera(float left, float right, float bottom, float top);

        void SetProjection(float left, float right, float bottom, float top);

        const FVec3& GetPosition() const { return m_Position; }
        void SetPosition(const FVec3& position)
        {
            m_Position = position;
            RecalculateViewMatrix();
        }

        float GetRotation() const { return m_Rotation; }
        void SetRotation(const float rotation)
        {
            m_Rotation = rotation;
            RecalculateViewMatrix();
        }


        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

        
    private:
        void RecalculateViewMatrix();

    private:
        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ViewProjectionMatrix;

        FVec3 m_Position = { 0.0f, 0.0f, 0.0f };
        float m_Rotation = 0.0f;
    };

    
}