#pragma once

#include "Haketon/Core/Core.h"
#include <rttr/type>

#include "Haketon/Scene/Entity.h"

namespace Haketon
{
    class HK_API ComponentRegistry
    {
    public:
        using AddFunc = std::function<void(Entity*)>;
        using RemoveFunc = std::function<void(Entity*)>;
        using GetFunc = std::function<rttr::variant(Entity*)>;
        using GetInstFunc = std::function<rttr::instance(Entity*)>;
        using SerializeFunc = std::function<void(Entity*, ISerializer*)>;
        using DeserializeFunc = std::function<void(Entity*, IDeserializer*, const std::string&)>;
        using HasFunc = std::function<bool(Entity*)>;

        struct ComponentInfo
        {
            ComponentInfo() : type(rttr::type::get<rttr::type>()) {}
            
            rttr::type type;
            AddFunc add;
            RemoveFunc remove;
            GetFunc get;
            GetInstFunc getInst;
            HasFunc has;
            SerializeFunc serialize;
            DeserializeFunc deserialize;
        };

        static ComponentRegistry& instance();

        template<typename T>
        void RegisterComponent(SerializeFunc ser, DeserializeFunc des)
        {
            ComponentInfo info;
            info.type = rttr::type::get<T>();

            info.add = [](Entity* entity)
            {
                entity->AddComponent<T>();
            };

            info.remove = [](Entity* entity)
            {
                entity->RemoveComponent<T>();
            };

            info.get = [](Entity* entity) -> rttr::variant
            {
                if (entity->HasComponent<T>())
                    return rttr::variant(entity->GetComponent<T>());
                return {};
            };

            info.getInst = [](Entity* entity) -> rttr::instance
            {
                if (entity->HasComponent<T>())
                    return rttr::instance(entity->GetComponent<T>());
                return {};
            };

            info.has = [](Entity* entity) -> bool
            {
                return entity->HasComponent<T>();
            };

            info.serialize = ser;
            info.deserialize = des;

            components[info.type] = info;
        }

        template<typename T>
        void UnregisterComponent()
        {
            rttr::type type = rttr::type::get<T>();
            auto it = components.find(type);
            if (it != components.end())
                components.erase(it);
        }

        const std::unordered_map<rttr::type, ComponentInfo>& GetAll() const
        {
            return components;
        }

        const ComponentInfo* GetComponentInfo(rttr::type t) const
        {
            auto it = components.find(t);
            return (it != components.end()) ? &it->second : nullptr;
        }

    private:
        std::unordered_map<rttr::type, ComponentInfo> components;
    };
}

