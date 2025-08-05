#pragma once

#include <entt/entt.hpp>

#include "Haketon/Core/Timestep.h"
#include <string>

#include "Haketon/Core/Misc/UUID.h"

namespace Haketon
{
    class EditorCamera;
    class Entity;
    struct Component;
    
    class Scene
    {
    public:
        Scene();
        Scene(const std::string& path, const std::string& name);
        ~Scene();

        Entity CreateEntity(const std::string& name = "Entity");
        void DestroyEntity(Entity entity);
        void DestroyAllEntities();
        Entity GetEntityByUUID(const FUUID& uuid);

        void OnUpdateRuntime(Timestep ts);
        void OnUpdateEditor(Timestep ts, EditorCamera& Camera);
        void OnViewportResize(uint32_t width, uint32_t height);
        Entity GetPrimaryCameraEntity();

        void SetGamePaused(bool paused) { m_IsGamePaused = paused; }

        bool IsTransient() const { return m_Path.empty(); }
        std::string GetPath() const { return m_Path; }
        std::string GetName() const { return m_Name; }
        
    private:

        void OnComponentAdded(Entity entity, Component* component);

        void SetPath(const std::string& path) { m_Path = path; }
        void SetName(const std::string& name) { m_Name = name; }
        
    private:
        entt::registry m_Registry;

        uint32_t m_ViewportWidth = 1, m_ViewportHeight = 1;

        bool m_IsGamePaused = false;

        std::string m_Path;
        std::string m_Name;

        friend class Entity;
        friend class SceneHierarchyPanel;
        friend class EditorLayer;

        friend class Serializer;
    };
}

