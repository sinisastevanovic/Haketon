#include "hkpch.h"
#include "Components.h"
#include "ScriptRegistry.h"

namespace Haketon
{
    void NativeScriptComponent::BindByName(const std::string& className)
    {
        ScriptClassName = className;
        UpdateBinding();
    }

    void NativeScriptComponent::UpdateBinding()
    {
        if (ScriptClassName.empty())
        {
            HK_CORE_WARN("NativeScriptComponent: ScriptClassName is empty");
            InstantiateScript = nullptr;
            DestroyScript = nullptr;
            return;
        }

        const auto& scripts = ScriptRegistry::Get().GetRegisteredScripts();
        
        for (const auto& script : scripts)
        {
            if (script.Name == ScriptClassName)
            {
                InstantiateScript = script.CreateInstance;
                DestroyScript = [](NativeScriptComponent* nsc) { 
                    delete nsc->Instance; 
                    nsc->Instance = nullptr; 
                };
                return;
            }
        }
        
        HK_CORE_ERROR("NativeScriptComponent: Script '{0}' not found in registry", ScriptClassName);
        InstantiateScript = nullptr;
        DestroyScript = nullptr;
    }
}