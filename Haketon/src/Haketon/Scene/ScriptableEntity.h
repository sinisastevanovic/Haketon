#pragma once

#include "Entity.h"
#include "Haketon/Core/Timestep.h"

#include <rttr/type>

#include "ScriptData.h"
#include "Haketon/Core/IReflectable.h"

namespace Haketon
{
    CLASS()
    class ScriptableEntity : public IReflectable
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
        
        void SetData(std::vector<ScriptData> data);
        Entity m_Entity;
        friend class Scene;
        friend class NativeScriptComponent;
        
        RTTR_ENABLE(IReflectable)
    };
}
