#pragma once

#include <rttr/type>
#include <functional>
#include <nlohmann/json.hpp>
#include <unordered_map>

#include "Haketon/Asset/AssetHandle.h"
#include "Haketon/Asset/AssetManager.h"

namespace Haketon
{
    class TypeHandlerRegistry
    {
    public:
        using CustomSerializerFn = std::function<nlohmann::json(const rttr::variant&)>;
        using CustomDeserializerFn = std::function<void(const nlohmann::json&, rttr::variant&)>;
        using AssetLoaderFn = std::function<rttr::variant(AssetHandle)>;

        static TypeHandlerRegistry& GetInstance();

        template <typename T>
        void RegisterHandlers(CustomSerializerFn serializer, CustomDeserializerFn deserializer);

        template <typename T>
        void RegisterAssetLoader();

        CustomSerializerFn FindSerializer(rttr::type type);
        CustomDeserializerFn FindDeserializer(rttr::type type);
        AssetLoaderFn FindAssetLoader(rttr::type type);

    private:
        TypeHandlerRegistry() = default;
        ~TypeHandlerRegistry() = default;

        TypeHandlerRegistry(const TypeHandlerRegistry&) = delete;
        TypeHandlerRegistry& operator=(const TypeHandlerRegistry&) = delete;
        TypeHandlerRegistry(TypeHandlerRegistry&&) = delete;
        TypeHandlerRegistry& operator=(TypeHandlerRegistry&&) = delete;

        CustomSerializerFn FindSerializerRecursive(rttr::type type);
        CustomDeserializerFn FindDeserializerRecursive(rttr::type type);

        std::unordered_map<rttr::type, CustomSerializerFn> m_Serializers;
        std::unordered_map<rttr::type, CustomDeserializerFn> m_Deserializers;
        std::unordered_map<rttr::type, AssetLoaderFn> m_AssetLoaders;

        std::unordered_map<rttr::type, rttr::type> m_SerializerCache;
        std::unordered_map<rttr::type, rttr::type> m_DeserializerCache;
    };

    template <typename T>
    void TypeHandlerRegistry::RegisterHandlers(CustomSerializerFn serializer, CustomDeserializerFn deserializer)
    {
        rttr::type type = rttr::type::get<T>();
        if (serializer)
        {
            m_Serializers[type] = std::move(serializer);
        }
        if (deserializer)
        {
            m_Deserializers[type] = std::move(deserializer);
        }

        m_SerializerCache.clear();
        m_DeserializerCache.clear();
    }

    template <typename T>
    void TypeHandlerRegistry::RegisterAssetLoader()
    {
        m_AssetLoaders[rttr::type::get<T>()] = [](AssetHandle handle) -> rttr::variant
        {
            return AssetManager::GetAsset<T>(handle);
        };
    }
}
