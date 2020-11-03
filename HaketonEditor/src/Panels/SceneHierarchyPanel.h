#pragma once

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
        SceneHierarchyPanel(const Ref<Scene>& scene);

        void SetContext(const Ref<Scene>& context);

        void OnImGuiRender();

    private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);
        
    private:
        Ref<Scene> m_Context;
        Entity m_SelectedEntity;
    };
}
