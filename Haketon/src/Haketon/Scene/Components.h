#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Haketon/Scene/ScriptableEntity.h"
#include "Haketon/Core/Core.h"

#include <rttr/type>
#include <functional>

//#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Haketon/Math/Math.h"


/*
 * TODO MetaData:
 *      - ReadOnly
 *      - ToolTip (Maybe automatically parse comment above property)
 *      - AdvancedDisplay (low prio)
 *      - Category (high prio)
 *      - HideFromParentObject (For example Camera in CameraComponent)
 *      
 */

// TODO: Need default objects to reset values to default

namespace Haketon
{
    STRUCT()
    struct Component
    {
    public:
        Component() = default;
        Component(const Component&) = default;
        virtual ~Component() = default;

        virtual void OnComponentDeserialized() { }
        
        RTTR_ENABLE()
    };

    STRUCT(NonRemovable)
    struct TransformComponent : Component
    {
        PROPERTY()
        FVec3 Position = { 0.0f, 0.0f, 0.0f };

        PROPERTY(Degrees)
        FVec3 Rotation = { 0.0f, 0.0f, 0.0f };

        PROPERTY()
        FVec3 Scale = { 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const FVec3& position)
            : Position(position) {}
        virtual ~TransformComponent() = default;

        glm::mat4 GetTransform() const
        {
            glm::mat4 rotation = glm::toMat4(FQuat(Rotation));
            
            return glm::translate(glm::mat4(1.0f), Position)
                * rotation
                * glm::scale(glm::mat4(1.0f), Scale);
        }

        RTTR_ENABLE(Component)
    };

    STRUCT(DisplayName="Sprite Renderer")
    struct SpriteRendererComponent : Component
    {
    public:
        PROPERTY()
        FColor Color { 1.0f, 1.0f, 1.0f, 1.0f };

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const FColor& color)
            : Color(color) {}
        virtual ~SpriteRendererComponent() = default;

        RTTR_ENABLE(Component)
    };

    STRUCT(DisplayName="Native Script")
    struct NativeScriptComponent : Component
    {
    public:

        NativeScriptComponent() = default;
        NativeScriptComponent(const NativeScriptComponent&) = default;
        virtual ~NativeScriptComponent() = default;

        void OnComponentDeserialized() override 
        {
            Component::OnComponentDeserialized(); // Call base implementation
            UpdateBinding(); // Update script binding after deserialization
        }

        PROPERTY()
        ScriptableEntity* Instance = nullptr;
        
        PROPERTY()
        std::string ScriptClassName = "";
    
        std::function<ScriptableEntity*()> InstantiateScript;
        std::function<void(NativeScriptComponent*)> DestroyScript;
        
        template<typename T>
        void Bind()
        {
            ScriptClassName = rttr::type::get<T>().get_name().to_string();
            InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
            DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };      
        }
        
        void BindByName(const std::string& className);
        void UpdateBinding();
        
        RTTR_ENABLE(Component)
    };
}