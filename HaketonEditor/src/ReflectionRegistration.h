#pragma once

#include <rttr/registration>
#include "Haketon/Scene/Components.h"

// TODO: find a way to have optional metadata. e.g. a "HasMetadata" function...
#define NOMETADATA metadata("Degrees", false)

namespace Haketon
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
    
        registration::class_<TagComponent>("TagComponent")
            .constructor()
            .property("Tag", &TagComponent::Tag)
                ( NOMETADATA );

        registration::class_<TransformComponent>("TransformComponent")
            .constructor()
            .property("Position", &TransformComponent::Position)
                ( NOMETADATA )
            .property("Rotation", &TransformComponent::Rotation)
                ( metadata("Degrees", true) )
            .property("Scale", &TransformComponent::Scale)
                ( NOMETADATA )
            .property("TestString", &TransformComponent::TestString)
                ( NOMETADATA );

        registration::class_<SpriteRendererComponent>("SpriteRendererComponent")
            .constructor()
            .property("Color", &SpriteRendererComponent::Color)
                ( NOMETADATA );

        registration::class_<CameraComponent>("CameraComponent")
            .constructor()
            .property("Primary", &CameraComponent::Primary)
                ( NOMETADATA )
            .property("Fixed Aspect Ration", &CameraComponent::FixedAspectRatio)
                ( NOMETADATA );
            //.property("Camera", &CameraComponent::Camera);

        registration::class_<SceneCamera>("SceneCamera")
            .constructor()
            .property("FOV", &SceneCamera::GetPerspectiveVerticalFOV, &SceneCamera::SetPerspectiveVerticalFOV)
                ( metadata("Degrees", true) )
            .property("Orthographic Size", &SceneCamera::GetOrthographicSize, &SceneCamera::SetOrthographicSize)
                ( NOMETADATA );
    }
}
