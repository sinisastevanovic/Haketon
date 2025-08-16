#pragma once

#include <entt/entt.hpp>

#include "Haketon/Core/Timestep.h"
#include <string>

#include "Haketon/Core/Misc/UUID.h"

#include <filesystem>

#include "Haketon/Asset/Asset.h"

namespace Haketon
{
    class EditorCamera;
    class Entity;
    struct Component;
    
    class HK_API Scene : public Asset
    {
    public:
        Scene() = default;
        Scene(const AssetHandle& handle, const std::string& path, const std::string& name);
        ~Scene();

        AssetType GetType() const override { return AssetType::Scene; }

        Entity CreateEntity(const std::string& name = "Entity");
        void DestroyEntity(Entity entity);
        void DestroyAllEntities();
        Entity GetEntityByUUID(const UUID& uuid);

        void OnUpdateRuntime(Timestep ts);
        void OnUpdateEditor(Timestep ts, EditorCamera& Camera);
        void OnViewportResize(uint32_t width, uint32_t height);
        Entity GetPrimaryCameraEntity();

        void SetPaused(bool paused) { m_IsPaused = paused; }
        bool IsPaused() const { return m_IsPaused; }

        static Ref<Scene> Copy(Scene* sceneToCopy);
        static Ref<Scene> Create(const std::filesystem::path& filePath, const AssetHandle& handle);
        static Ref<Scene> Open(const std::filesystem::path& filePath);
        
    private:

        void OnComponentAdded(Entity entity, Component* component);

    private:
        entt::registry m_Registry;

        uint32_t m_ViewportWidth = 1, m_ViewportHeight = 1;

        bool m_IsPaused = false;

        friend class Entity;
        friend class SceneHierarchyPanel;
        friend class EditorLayer;
        friend class HaketonEditor;

        friend class RapidJsonSerializer;
    };
}

