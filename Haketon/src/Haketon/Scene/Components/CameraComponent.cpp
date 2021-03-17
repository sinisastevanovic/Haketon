#include "hkpch.h"
#include "CameraComponent.h"
#include "Haketon/Scene/SceneCamera.h"

namespace Haketon
{
    Haketon::CameraComponent::CameraComponent()
    {
        Camera = CreateRef<SceneCamera>();
    }
}

