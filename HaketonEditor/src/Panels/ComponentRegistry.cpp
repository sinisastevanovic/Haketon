#include "ComponentRegistry.h"

#include "Haketon/Scene/Components.h"

namespace Haketon
{
    // Get all component types from reflection
    std::vector<rttr::type> GetAllComponentTypesFromReflection()
    {
        std::vector<rttr::type> componentTypes;
        
        rttr::type baseComponentType = rttr::type::get<Component>();
        for (auto& type : rttr::type::get_types())
        {
            if (!type.is_valid() || type == baseComponentType)
                continue;
                
            if (type.is_derived_from(baseComponentType))
            {
                componentTypes.push_back(type);
            }
        }
        
        return componentTypes;
    }

    // This will be populated by the SceneHierarchyPanel
    static std::vector<ComponentInfo> s_ComponentRegistry;
    static bool s_RegistryInitialized = false;

    // Initialize component registry with callbacks from SceneHierarchyPanel
    void InitializeComponentRegistry(const std::vector<ComponentInfo>& registry)
    {
        s_ComponentRegistry = registry;
        s_RegistryInitialized = true;
    }

    // Get the component registry
    const std::vector<ComponentInfo>& GetComponentRegistry()
    {
        return s_ComponentRegistry;
    }

    // Check if registry is initialized
    bool IsComponentRegistryInitialized()
    {
        return s_RegistryInitialized;
    }
}