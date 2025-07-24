#pragma once
#include "Haketon/Scene/Components.h"
#include "Panels/DetailCustomization/ColorDetailCustomization.h"

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
        FVec2 UV = { 0.0f, 0.0f };

        PROPERTY()
        FVec3 Position = { 0.0f, 0.0f, 0.0f };

        PROPERTY()
        FVec4 Vec4 = { 1.0f, 1.0f, 1.0f, 1.0f };

        PROPERTY()
        FColor Color = { 1.0f, 1.0f, 1.0f, 1.0f };

        PROPERTY()
        glm::ivec2 Velocity = { 0.0f, 0.0f };

        PROPERTY()
        glm::ivec3 Rotation = { 0.0f, 0.0f, 0.0f };

        PROPERTY()
        glm::ivec4 Test = { 0.0f, 0.0f, 0.0f, 0.0f };

        PROPERTY()
        glm::quat TestQuaternion = { 0.0f, 0.0f, 0.0f, 0.0f };
        
        PROPERTY() 
        float Mass = 1.0f;

        PROPERTY(EditCondition = "Mass > 2.0f")
        bool TestBool = false;

        PROPERTY(EditCondition = "TestBool")
        bool TestBool2 = false;

        PROPERTY(EditCondition = "TestBool && TestBool2")
        int TestInt = 0;

        PROPERTY(VisibleCondition = "(TestBool && TestBool2) || (Mass < 2.0f && Mass > 1.0f)")
        float TestFloat = 0.0f;

        PROPERTY()
        std::vector<int> TestIntVec;

        PROPERTY()
        std::map<std::string, int> TestMap;

        PROPERTY()
        int TestArray[3];

        PROPERTY()
        bool UseGravity = true;

        PhysicsComponent() = default;
        PhysicsComponent(const PhysicsComponent&) = default;
        virtual ~PhysicsComponent() = default;

        RTTR_ENABLE(Component)
    };
}
