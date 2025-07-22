#pragma once
#include "Haketon/Scene/Components.h"

namespace Haketon
{
    STRUCT()
    struct TestComponent : Component
    {
    public:
        PROPERTY()
        std::string Tag;
        
        PROPERTY()
        int TestInt;

        TestComponent() = default;
        TestComponent(const TestComponent&) = default;
        TestComponent(const std::string tag, const int testInt)
            : Tag(tag), TestInt(testInt) {}
        virtual ~TestComponent() = default;

        RTTR_ENABLE(Component)
    };
}
