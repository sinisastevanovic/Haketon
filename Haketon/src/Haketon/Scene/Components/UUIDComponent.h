#pragma once
#include "Haketon/Core/Misc/UUID.h"
#include "Haketon/Scene/Components.h"

namespace Haketon 
{
    STRUCT(NonRemovable, Hidden)
    struct HK_API UUIDComponent : Component
    {
        UUIDComponent() : Uuid() {}

        // For deserlialization
        explicit UUIDComponent(UUID existingUuid) : Uuid(std::move(existingUuid)) {}
        
        PROPERTY()
        UUID Uuid;
        
        RTTR_ENABLE(Component)
    };
}
