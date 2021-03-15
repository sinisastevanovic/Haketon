#pragma once

#include <rttr/registration>
#include "Haketon/Scene/Components.h"

// TODO: find a way to have optional metadata. e.g. a "HasMetadata" function...
#define NOMETADATA //metadata("Degrees", false)

namespace Haketon
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
    
        registration::class_<TagComponent>("TagComponent")
            .constructor()
            .property("Tag", &TagComponent::Tag);

        registration::class_<TransformComponent>("TransformComponent")
            .constructor()
            .property("Position", &TransformComponent::Position)             
            .property("Rotation", &TransformComponent::Rotation)
                ( metadata("Degrees", true) )
            .property("Scale", &TransformComponent::Scale)               
            .property("TestString", &TransformComponent::TestString);

        registration::class_<SpriteRendererComponent>("SpriteRendererComponent")
            .constructor()
            .property("Color", &SpriteRendererComponent::Color);

        registration::class_<CameraComponent>("CameraComponent")
            .constructor()
            .property("Primary", &CameraComponent::Primary)                                         
            .property("Fixed Aspect Ration", &CameraComponent::FixedAspectRatio)
            .property("Camera", &CameraComponent::Camera)
            ;

        registration::class_<SceneCamera>("SceneCamera")
            .constructor()
            .property("ProjectionType", &SceneCamera::GetProjectionType, &SceneCamera::SetProjectionType)
            .property("FOV", &SceneCamera::GetPerspectiveVerticalFOV, &SceneCamera::SetPerspectiveVerticalFOV)
                ( metadata("Degrees", true) )
            .property("Perspective Near", &SceneCamera::GetPerspectiveNearClip, &SceneCamera::SetPerspectiveNearClip)
            .property("Perspective Far", &SceneCamera::GetPerspectiveFarClip, &SceneCamera::SetPerspectiveFarClip)
            .property("Orthographic Size", &SceneCamera::GetOrthographicSize, &SceneCamera::SetOrthographicSize)
            .property("Orthographic Near", &SceneCamera::GetOrthographicNearClip, &SceneCamera::SetOrthographicNearClip)
            .property("Orthographic Far", &SceneCamera::GetOrthographicFarClip, &SceneCamera::SetOrthographicFarClip)
                ;

        registration::enumeration<SceneCamera::ProjectionType>("ProjectionType")
        (
            value("Orthographic", SceneCamera::ProjectionType::Orthographic),    
            value("Perspective", SceneCamera::ProjectionType::Perspective)    
        );
        
        registration::class_<TestClass>("Test")
            .constructor()(rttr::policy::ctor::as_object)
            .property("TestFloat", &TestClass::TestFloat);    
    }
}
