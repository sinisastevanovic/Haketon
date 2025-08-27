#pragma once
#include "Haketon/Core/Core.h"
#include "Haketon/Scene/Components.h"

namespace Haketon
{
    struct HK_API ParentComponent : Component
    {
        ParentComponent(entt::entity parent, uint32_t depth) : Parent(parent), Depth(depth) {}
        entt::entity Parent;
        uint32_t Depth = 0;
    };
}