#include "hkpch.h"
#include "Components.h"
#include "ScriptRegistry.h"

namespace Haketon
{
    void NativeScriptComponent::OnPropertyChanged(const std::string& propertyName)
    {
        HK_CORE_INFO("NativeScriptComponent: Property '{0}' changed", propertyName);
        if (propertyName == "ScriptClassName")
        {
            UpdateBinding();
        }
        else if (propertyName == "Data")
        {
            if (Instance != nullptr)
            {
                Instance->SetData(Data);
            }
        }
    }

    void NativeScriptComponent::BindByName(const std::string& className)
    {
        ScriptClassName = className;
        UpdateBinding();
    }

    void NativeScriptComponent::UpdateBinding()
    {
        if (Instance)
        {
            delete Instance;
            Instance = nullptr;
        }
        
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
                if (script.Data.size() == Data.size() && Data.size() > 0 && Data[0].ScriptName == script.Name)
                {
                    // Keep data
                }
                else
                {
                    Data = script.Data;
                }
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