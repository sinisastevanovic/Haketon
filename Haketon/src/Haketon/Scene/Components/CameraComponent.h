#pragma once
#include "Haketon/Scene/Components.h"
#include <vector>

namespace Haketon
{
    class SceneCamera;

    STRUCT()
    struct CameraComponent : Component
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

