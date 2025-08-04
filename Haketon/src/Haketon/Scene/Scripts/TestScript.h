#pragma once
#include <Haketon.h>

namespace Haketon
{
    STRUCT()
    struct ExampleScript : ScriptableEntity
    {
    public:
        void OnCreate() override
        {
            HK_CORE_INFO("ExampleScript created!");
        }

        void OnUpdate(Timestep ts) override
        {
            auto& transform = GetComponent<TransformComponent>();
            transform.Position.x += 1.0f * ts;
        }

        void OnDestroy() override
        {
            HK_CORE_INFO("ExampleScript destroyed!");
        }
    
        RTTR_ENABLE(ScriptableEntity)
    };
}
