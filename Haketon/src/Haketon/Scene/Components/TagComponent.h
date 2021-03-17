#pragma once
#include "Haketon/Scene/Components.h"

namespace Haketon
{
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
}

