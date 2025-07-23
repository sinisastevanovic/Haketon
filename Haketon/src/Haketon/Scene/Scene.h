﻿#pragma once

#include <entt.hpp>

#include "Haketon/Core/Timestep.h"
#include <string>

namespace Haketon
{
    class EditorCamera;
    class Entity;
    struct Component;
    
    class Scene
    {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const std::string& name = "Entity");
        void DestroyEntity(Entity entity);
        void DestroyAllEntities();

        void OnUpdateRuntime(Timestep ts);
        void OnUpdateEditor(Timestep ts, EditorCamera& Camera);
        void OnViewportResize(uint32_t width, uint32_t height);
        Entity GetPrimaryCameraEntity();
    private:

        void OnComponentAdded(Entity entity, Component* component);
        
    private:
        entt::registry m_Registry;

        uint32_t m_ViewportWidth = 1, m_ViewportHeight = 1;

        friend class Entity;
        friend class SceneHierarchyPanel;

        friend class Serializer;
    };
}

