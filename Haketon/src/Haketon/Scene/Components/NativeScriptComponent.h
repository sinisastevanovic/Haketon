#pragma once
#include "Haketon/Scene/Components.h"
#include "Haketon/Scene/ScriptableEntity.h"
#include "Haketon/Scene/ScriptData.h"

namespace Haketon
{
    STRUCT(DisplayName="Native Script")
    struct HK_API NativeScriptComponent : Component
    {
    public:

        NativeScriptComponent() = default;
        NativeScriptComponent(const NativeScriptComponent& other) 
            : Component(other)
            , ScriptClassName(other.ScriptClassName)
            , Data(other.Data)
            , InstantiateScript(nullptr)
            , DestroyScript(nullptr)
            , Instance(nullptr) // Don't copy the instance pointer - avoid double deletion
        {
        }
        virtual ~NativeScriptComponent() 
        {
            // Clean up script instance when component is destroyed
            if (Instance && DestroyScript)
            {
                DestroyScript(this);
            }
        }

        void OnComponentDeserialized() override 
        {
            Component::OnComponentDeserialized(); // Call base implementation
            UpdateBinding(); // Update script binding after deserialization
        }

        void OnPropertyChanged(const std::string& propertyName) override;
    
        ScriptableEntity* Instance = nullptr;
        
        PROPERTY(ScriptSelector)
        std::string ScriptClassName = "";

        PROPERTY()
        std::vector<ScriptData> Data;
    
        std::function<ScriptableEntity*()> InstantiateScript;
        std::function<void(NativeScriptComponent*)> DestroyScript;
        
        template<typename T>
        void Bind()
        {
            ScriptClassName = rttr::type::get<T>().get_name().to_string();
            InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
            DestroyScript = [](NativeScriptComponent* nsc)
            {
                if (nsc->Instance)
                {
                    nsc->Instance->OnDestroy();
                    delete nsc->Instance;
                    nsc->Instance = nullptr;
                }
            };      
        }
        
        void BindByName(const std::string& className);
        void UpdateBinding();
        
        RTTR_ENABLE(Component)
    };
}
