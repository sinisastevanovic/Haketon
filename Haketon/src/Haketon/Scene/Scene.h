#pragma once

#include <entt.hpp>

#include "Haketon/Core/Timestep.h"

namespace Haketon
{
    class Entity;
    
    class Scene
    {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const std::string& name = "Entity");

        void OnUpdate(Timestep ts);

    private:
        entt::registry m_Registry;

        friend class Entity;
    };
}

