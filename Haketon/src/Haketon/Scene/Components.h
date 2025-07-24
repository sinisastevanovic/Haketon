#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Haketon/Scene/ScriptableEntity.h"
#include "Haketon/Core/Core.h"

#include <rttr/type>

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
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
            
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