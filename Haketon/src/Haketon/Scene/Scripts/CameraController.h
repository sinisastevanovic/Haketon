#pragma once
#include <Haketon.h>

namespace Haketon
{
    STRUCT()
    struct CameraController : ScriptableEntity
    {
    public:
        PROPERTY()
        float Speed = 1.0f;
        
        void OnCreate() override
        {
        }

        void OnUpdate(Timestep ts) override
        {
            auto& transform = GetComponent<TransformComponent>();
            if (Input::IsKeyPressed(Key::A))
            {
                transform.Position.x += 1.0f * ts;
            }
        }

        void OnDestroy() override
        {
        }
    
        RTTR_ENABLE(ScriptableEntity)
    };
}
