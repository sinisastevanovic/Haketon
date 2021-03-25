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
        PROPERTY()
        char Char = 'L';
        PROPERTY()
        int8_t Int8 = 0;
        PROPERTY()
        int16_t Int16 = 0;
        PROPERTY()
        int32_t Int32 = 0;
        PROPERTY()
        float Float = 0;
        PROPERTY()
        double Double = 0;
        PROPERTY()
        std::string String = "Hi";

        PROPERTY()
        std::vector<int> IntArray = { 12, 13, 4, 788, 19829 };

        PROPERTY()
        std::vector<std::string> StringArray = { "12", "13", "4", "788", "19829" };

        RTTR_ENABLE(Component)
    };
}

