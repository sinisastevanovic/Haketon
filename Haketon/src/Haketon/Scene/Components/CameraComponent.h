#pragma once
#include "Haketon/Core/Core.h"
#include "Haketon/Scene/Components.h"
#include "Haketon/Scene/SceneCamera.h"
#include <vector>

namespace Haketon
{
    STRUCT()
    struct HK_API CameraComponent : Component
    {
        CameraComponent(); 
        CameraComponent(const CameraComponent& other) = default;

        virtual ~CameraComponent() = default;

        PROPERTY()
        Ref<SceneCamera> Camera;

        // If true, this acts as the primary camera for the scene
        PROPERTY()
        bool Primary = false; // TODO: move this to scene
        PROPERTY()
        bool FixedAspectRatio = false;       

        RTTR_ENABLE(Component)
    };
}

