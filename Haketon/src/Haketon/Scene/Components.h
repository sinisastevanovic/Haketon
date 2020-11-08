#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Haketon/Scene/SceneCamera.h"
#include "Haketon/Scene/ScriptableEntity.h"

#include <rttr/type>



namespace Haketon
{  
    struct TagComponent
    {
    public:
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string tag)
            : Tag(tag) {}
        virtual ~TagComponent() = default;

        RTTR_ENABLE()
    };
    
    struct TransformComponent
    {
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
        std::string TestString = "hurensohn";

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& position)
            : Position(position) {}
        virtual ~TransformComponent() = default;

        glm::mat4 GetTransform() const
        {
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, {1, 0, 0})
                * glm::rotate(glm::mat4(1.0f), Rotation.y, {0, 1, 0})
                * glm::rotate(glm::mat4(1.0f), Rotation.z, {0, 0, 1});
            
            return glm::translate(glm::mat4(1.0f), Position)
                * rotation
                * glm::scale(glm::mat4(1.0f), Scale);
        }

        RTTR_ENABLE()
    };

    struct SpriteRendererComponent
    {
    public:
        glm::vec4 Color { 1.0f, 1.0f, 1.0f, 1.0f };

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color)
            : Color(color) {}
        virtual ~SpriteRendererComponent() = default;

        RTTR_ENABLE()
    };

    struct CameraComponent
    {
        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
        virtual ~CameraComponent() = default;
        
        SceneCamera Camera;
        bool Primary = false; // TODO: move this to scene
        bool FixedAspectRatio = false;

        RTTR_ENABLE()
    };

    struct NativeScriptComponent
    {
        ScriptableEntity* Instance = nullptr;
    
        ScriptableEntity*(*InstantiateScript)();
        void (*DestroyScript)(NativeScriptComponent*);
        
        template<typename T>
        void Bind()
        {
            InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
            DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };      
        }
    };

    /*struct TestComponent
    {
    public:
        TestComponent() = default;
        virtual ~TestComponent();

        float TestFloat = 1.0f;

       // RTTR_ENABLE()
    };*/
}