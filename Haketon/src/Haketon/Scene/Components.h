#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Haketon/Scene/SceneCamera.h"
#include "Haketon/Scene/ScriptableEntity.h"
#include "Haketon/Core/Core.h"

#include <rttr/type>

// BIG TODO: I don't want to call CreateComponentSection manually in SceneHierarchyPanel

namespace Haketon
{
    STRUCT()
    struct Component
    {
    public:
        Component() = default;
        Component(const Component&) = default;
        virtual ~Component() = default;
        
        RTTR_ENABLE()
    };
    
    STRUCT()
    struct TagComponent : Component
    {
    public:
        PROPERTY()
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string tag)
            : Tag(tag) {}
        virtual ~TagComponent() = default;

        RTTR_ENABLE(Component)
    };

    STRUCT()
    struct IntComponent : Component
    {
    public:
        IntComponent() = default;
        IntComponent(const IntComponent&) = default;

        PROPERTY(DisplayName=Penis, HideInDetails)
        int IntProperty = 42;

        RTTR_ENABLE(Component)
    };

    STRUCT()
    struct TransformComponent : Component
    {
        PROPERTY()
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };

        PROPERTY(Degrees)
        glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };

        PROPERTY()
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

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

        RTTR_ENABLE(Component)
    };

    STRUCT()
    struct SpriteRendererComponent : Component
    {
    public:
        PROPERTY()
        glm::vec4 Color { 1.0f, 1.0f, 1.0f, 1.0f };

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color)
            : Color(color) {}
        virtual ~SpriteRendererComponent() = default;

        RTTR_ENABLE(Component)
    };

    STRUCT()
    struct CameraComponent : Component
    {
        CameraComponent() { Camera = CreateRef<SceneCamera>(); }
        CameraComponent(const CameraComponent& other) = default;

        virtual ~CameraComponent() = default;

        PROPERTY()
        Ref<SceneCamera> Camera;
        PROPERTY()
        bool Primary = false; // TODO: move this to scene
        PROPERTY()
        bool FixedAspectRatio = false;

        RTTR_ENABLE(Component)
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
}