#include "hkpch.h"
#include "TypeHandlerRegistry.h"

namespace Haketon
{
    TypeHandlerRegistry& TypeHandlerRegistry::GetInstance()
    {
        static TypeHandlerRegistry instance;
        return instance;
    }

    TypeHandlerRegistry::CustomSerializerFn TypeHandlerRegistry::FindSerializer(rttr::type type)
    {
        auto cacheIt = m_SerializerCache.find(type);
        if (cacheIt != m_SerializerCache.end())
        {
            return m_Serializers.at(cacheIt->second);
        }

        return FindSerializerRecursive(type);
    }

    TypeHandlerRegistry::CustomSerializerFn TypeHandlerRegistry::FindSerializerRecursive(rttr::type type)
    {
        if (!type.is_valid())
            return nullptr;

        auto it = m_Serializers.find(type);
        if (it != m_Serializers.end())
        {
            m_SerializerCache.emplace(type, type);
            return it->second;
        }

        for (const auto& baseType : type.get_base_classes())
        {
            CustomSerializerFn handler = FindSerializerRecursive(baseType);
            if (handler)
            {
                rttr::type handlerProviderType = m_SerializerCache.at(baseType);
                m_SerializerCache.emplace(type, handlerProviderType);
                return handler;
            }
        }

        return nullptr;
    }

    TypeHandlerRegistry::CustomDeserializerFn TypeHandlerRegistry::FindDeserializer(rttr::type type)
    {
        auto cacheIt = m_DeserializerCache.find(type);
        if (cacheIt != m_DeserializerCache.end())
        {
            return m_Deserializers.at(cacheIt->second);
        }
        
        return FindDeserializerRecursive(type);
    }

    TypeHandlerRegistry::AssetLoaderFn TypeHandlerRegistry::FindAssetLoader(rttr::type type)
    {
        auto it = m_AssetLoaders.find(type);
        if (it != m_AssetLoaders.end())
        {
            return it->second;
        }
        return nullptr;
    }

    TypeHandlerRegistry::CustomDeserializerFn TypeHandlerRegistry::FindDeserializerRecursive(rttr::type type)
    {
        if (!type.is_valid())
            return nullptr;

        auto it = m_Deserializers.find(type);
        if (it != m_Deserializers.end())
        {
            m_DeserializerCache.emplace(type, type);
            return it->second;
        }

        for (const auto& baseType : type.get_base_classes())
        {
            CustomDeserializerFn handler = FindDeserializerRecursive(baseType);
            if (handler)
            {
                rttr::type handlerProviderType = m_DeserializerCache.at(baseType);
                m_DeserializerCache.emplace(type, handlerProviderType);
                return handler;
            }
        }

        return nullptr;
    }
}
