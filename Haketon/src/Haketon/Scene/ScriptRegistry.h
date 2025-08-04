#pragma once

#include <vector>
#include <string>
#include <functional>
#include <rttr/type>
#include "ScriptableEntity.h"
#include "Haketon/Core/Log.h"

namespace Haketon
{
    struct ScriptInfo
    {
        std::string Name;
        rttr::type Type;
        std::function<ScriptableEntity*()> CreateInstance;
        
        ScriptInfo() : Type(rttr::type::get<void>()) {}
        
        ScriptInfo(const std::string& name, rttr::type type, std::function<ScriptableEntity*()> createFunc)
            : Name(name), Type(type), CreateInstance(createFunc) {}
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
            
            // Get the base ScriptableEntity type
            rttr::type baseType = rttr::type::get<ScriptableEntity>();
            
            // Debug: Log the base type
            HK_CORE_INFO("ScriptRegistry: Looking for types derived from {0}", baseType.get_name().to_string());
            
            // Iterate through all registered types
            for (auto& type : rttr::type::get_types())
            {
                // Check if this type derives from ScriptableEntity
                if (type.is_derived_from(baseType) && type != baseType)
                {
                    HK_CORE_INFO("ScriptRegistry: Found derived type {0}", type.get_name().to_string());
                    
                    // Only register concrete types (not abstract)
                    if (type.get_constructor().is_valid())
                    {
                        HK_CORE_INFO("ScriptRegistry: Registering script {0}", type.get_name().to_string());
                        
                        // Create lambda that uses RTTR to instantiate
                        auto createFunc = [type]() -> ScriptableEntity* {
                            rttr::variant obj = type.create();
                            if (obj.is_valid())
                            {
                                return obj.get_value<ScriptableEntity*>();
                            }
                            return nullptr;
                        };
                        
                        m_Scripts.emplace_back(type.get_name().to_string(), type, createFunc);
                    }
                    else
                    {
                        HK_CORE_WARN("ScriptRegistry: Type {0} has no valid constructor", type.get_name().to_string());
                    }
                }
            }
            
            HK_CORE_INFO("ScriptRegistry: Registered {0} script types", m_Scripts.size());
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