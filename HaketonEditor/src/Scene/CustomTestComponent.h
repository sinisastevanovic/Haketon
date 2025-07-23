#pragma once
#include "Haketon/Scene/Components.h"

namespace Haketon
{
    STRUCT(DisplayName="My Custom Component")
    struct CustomTestComponent : Component
    {
    public:
        PROPERTY()
        std::string Name = "Default Name";
        
        PROPERTY()
        int Value = 42;

        PROPERTY()
        bool IsEnabled = true;

        CustomTestComponent() = default;
        CustomTestComponent(const CustomTestComponent&) = default;
        virtual ~CustomTestComponent() = default;

        RTTR_ENABLE(Component)
    };

    STRUCT(DisplayName="Physics Body")
    struct PhysicsComponent : Component
    {
    public:
        PROPERTY() 
        float Mass = 1.0f;
        
        PROPERTY()
        glm::vec3 Velocity = { 0.0f, 0.0f, 0.0f };

        PROPERTY()
        bool UseGravity = true;

        PhysicsComponent() = default;
        PhysicsComponent(const PhysicsComponent&) = default;
        virtual ~PhysicsComponent() = default;

        RTTR_ENABLE(Component)
    };
}