#pragma once
#include <Haketon.h>

namespace Haketon
{
    STRUCT()
    struct HK_API CameraController : ScriptableEntity
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
                transform.Position.x += Speed * ts;
            }
            else if (Input::IsKeyPressed(Key::D))
            {
                transform.Position.x -= Speed * ts;
            }
        }

        void OnDestroy() override
        {
        }
    
        RTTR_ENABLE(ScriptableEntity)
    };
}
