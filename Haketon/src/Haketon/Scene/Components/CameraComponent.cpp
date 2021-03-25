#include "hkpch.h"
#include "CameraComponent.h"
#include "Haketon/Scene/SceneCamera.h"

#include <rttr/type>

namespace Haketon
{
    Haketon::CameraComponent::CameraComponent()
    {
        Camera = CreateRef<SceneCamera>();
    }
}

