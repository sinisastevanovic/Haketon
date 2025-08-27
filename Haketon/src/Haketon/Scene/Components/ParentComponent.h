#pragma once
#include "Haketon/Core/Core.h"
#include "Haketon/Scene/Components.h"

namespace Haketon
{
    STRUCT(Hidden)
    struct HK_API ParentComponent : Component
    {
        ParentComponent() = default;
        ParentComponent(entt::entity parent, uint32_t depth, const UUID& parentHandle) : Parent(parent), Depth(depth), ParentHandle(parentHandle) {}

        entt::entity Parent;

        PROPERTY()
        uint32_t Depth = 0;

        PROPERTY()
        UUID ParentHandle = UUID::Null();

        RTTR_ENABLE(Component)
    };
}