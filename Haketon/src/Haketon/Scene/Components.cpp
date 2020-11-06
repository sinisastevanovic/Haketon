#include "hkpch.h"
#include "Components.h"

#include <rttr/registration>

RTTR_REGISTRATION
{
    using namespace rttr;
    using namespace Haketon;

    registration::class_<TestComponent>("TestComponent")
        .property("TestFloat", &TestComponent::TestFloat)
        (
            metadata("TOOL TIP", "This is a test float")
        );
}