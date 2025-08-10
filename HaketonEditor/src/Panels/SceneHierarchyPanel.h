#pragma once

#include "Haketon/Core/ComponentRegistry.h"
#include "Haketon/Core/Log.h"
#include "Haketon/Core/Core.h"
#include "Haketon/Scene/Entity.h"


namespace Haketon
{
    class Scene;

    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(Scene* scene);

        void SetContext(Scene* context);

        void OnImGuiRender();

        Entity GetSelectedEntity() const { return m_SelectedEntity; }
        void SetSelectedEntity(Entity Entity) { m_SelectedEntity = Entity; }

    private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);

        void CreateComponentSection(Entity entity, const ComponentRegistry::ComponentInfo* info);
        
    private:
        Scene* m_Context;
        Entity m_SelectedEntity;

    public:
        static float minRowHeight;
        static int CurrentIndentation;

        static bool bAddVarsInititalized;
    };
}
