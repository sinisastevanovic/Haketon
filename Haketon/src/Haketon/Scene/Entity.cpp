#include "hkpch.h"
#include "Entity.h"

namespace Haketon
{
    Entity::Entity(entt::entity handle, Scene* scene)
        : m_EntityHandle(handle), m_Scene(scene)
    {
    }
}
