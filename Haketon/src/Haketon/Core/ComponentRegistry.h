#pragma once
#include <entt/entt.hpp>
#include <rttr/type>

#include "Haketon/Scene/Entity.h"

namespace Haketon
{
    class ComponentRegistry
    {
    public:
        using AddFunc = std::function<void(Entity*)>;
        using GetFunc = std::function<rttr::variant(Entity*)>;
        using SerializeFunc = std::function<void(Entity*, ISerializer*)>;
        using DeserializeFunc = std::function<void(Entity*, IDeserializer*, const std::string&)>;
        using HasFunc = std::function<bool(Entity*)>;

        struct ComponentInfo
        {
            ComponentInfo() : type(rttr::type::get<rttr::type>()) {}
            
            rttr::type type;
            AddFunc add;
            GetFunc get;
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

            info.get = [](Entity* entity) -> rttr::variant
            {
                if (entity->HasComponent<T>())
                    return rttr::variant(entity->GetComponent<T>());
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

