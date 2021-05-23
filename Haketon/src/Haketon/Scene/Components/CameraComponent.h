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
        PROPERTY()
        bool Primary = false; // TODO: move this to scene
        PROPERTY()
        bool FixedAspectRatio = false;       

        RTTR_ENABLE(Component)
    };
}

