#include "hkpch.h"
#include "Entity.h"

namespace Haketon
{
    Entity::Entity(entt::entity handle, Scene* scene)
        : m_EntityHandle(handle), m_Scene(scene)
    {
    }

    /*template <typename T, typename ... Args>
    T& Entity::AddComponent(Args&&... args)
    {
        HK_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
        T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);

        Component* castComp = dynamic_cast<Component>(&component);
        if(castComp)
            m_Scene->OnComponentAdded(*this, castComp);
        
        return component;
    }*/
}
