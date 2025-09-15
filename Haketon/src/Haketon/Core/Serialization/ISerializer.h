#pragma once

#include <rttr/variant.h>
#include <string>

namespace Haketon
{
    class HK_API ISerializerNew
    {
    public:
        virtual ~ISerializerNew() = default;
        virtual std::string Serialize(const rttr::variant& object) = 0;
    };

    class HK_API IDeserializerNew
    {
    public:
        virtual ~IDeserializerNew() = default;
        virtual bool DeserializeVar(const std::string& data, rttr::variant& object) = 0;

        template<typename T>
        bool Deserialize(const std::string& data, T& object)
        {
            rttr::variant var = std::ref(object);
            return DeserializeVar(data, var);
        }

        template<typename T>
        std::optional<T> Deserialize(const std::string& data)
        {
            static_assert(std::is_default_constructible_v<T>, "Type must be default constructible by-value.");
            T tempObject;

            if (Deserialize(data, tempObject))
            {
                return tempObject;
            }

            return std::nullopt;
        }
    };
}