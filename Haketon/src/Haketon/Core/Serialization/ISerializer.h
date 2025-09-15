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
        virtual bool Deserialize(const std::string& data, rttr::variant& object) = 0;
    };
}