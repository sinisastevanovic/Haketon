#pragma once

#include <vector>
#include <string>
#include <functional>
#include <rttr/type>
#include "ScriptableEntity.h"
#include "ScriptData.h"
#include "Haketon/Core/Log.h"

namespace Haketon
{
    // Forward declaration for generated function
    void RegisterAllGeneratedScripts();

    struct ScriptInfo
    {
        std::string Name;
        rttr::type Type;
        std::function<ScriptableEntity*()> CreateInstance;
        std::vector<ScriptData> Data;
        
        ScriptInfo() : Type(rttr::type::get<void>()) {}
        
        ScriptInfo(const std::string& name, rttr::type type, std::function<ScriptableEntity*()> createFunc, std::vector<ScriptData> scriptData)
            : Name(name), Type(type), CreateInstance(createFunc), Data(scriptData) {}
    };

    class ScriptRegistry
    {
    public:
        static ScriptRegistry& Get()
        {
            static ScriptRegistry instance;
            return instance;
        }

        void Initialize()
        {
            m_Scripts.clear();
            
            // Call auto-generated script registration
            RegisterAllGeneratedScripts();
            
            HK_CORE_INFO("ScriptRegistry: Registered {0} script types", m_Scripts.size());
        }
        
        template<typename T>
        void RegisterScript(const std::string& name)
        {
            static_assert(std::is_base_of_v<ScriptableEntity, T>, "T must derive from ScriptableEntity");
            
            auto createFunc = []() -> ScriptableEntity* {
                return new T();
            };
            
            rttr::type type = rttr::type::get<T>();
            auto defaultObject = type.create();
            std::vector<ScriptData> scriptData;
            for (auto prop : type.get_properties())
            {
                ScriptData data;
                data.ScriptName = name;
                data.Name = prop.get_name().to_string();
                data.Type = prop.get_type().get_name().to_string();
                data.Value = prop.get_value(defaultObject);
                scriptData.emplace_back(data);
            }
            
            m_Scripts.emplace_back(name, type, createFunc, scriptData);
        }

        const std::vector<ScriptInfo>& GetRegisteredScripts() const { return m_Scripts; }
        
        ScriptableEntity* CreateScript(const std::string& name) const
        {
            for (const auto& script : m_Scripts)
            {
                if (script.Name == name)
                {
                    return script.CreateInstance();
                }
            }
            return nullptr;
        }

        bool IsValidScript(const std::string& name) const
        {
            for (const auto& script : m_Scripts)
            {
                if (script.Name == name)
                    return true;
            }
            return false;
        }

    private:
        std::vector<ScriptInfo> m_Scripts;
    };

}