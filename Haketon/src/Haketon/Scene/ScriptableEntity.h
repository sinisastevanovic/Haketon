#pragma once

#include "Entity.h"
#include "Haketon/Core/Timestep.h"

#include <rttr/type>

#include "Haketon/Core/IReflectable.h"

namespace Haketon
{
    STRUCT()
    struct ScriptableEntity : IReflectable
    {
    public:

        virtual ~ScriptableEntity() {}
        
        template<typename T>
        T& GetComponent()
        {
            return m_Entity.GetComponent<T>();
        }

    protected:
        virtual void OnCreate() {}
        virtual void OnDestroy() {} // TODO: This never gets called when the scene is being destroyed!
        virtual void OnUpdate(Timestep ts) {}

    private:
        Entity m_Entity;
        friend class Scene;
        
        RTTR_ENABLE(IReflectable)
    };
}
