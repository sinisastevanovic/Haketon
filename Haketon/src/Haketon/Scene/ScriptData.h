#pragma once

#include "Haketon/Core/Serialization/ISerializationContext.h"
#include <rttr/variant.h>
#include <rttr/type>

namespace Haketon
{
    STRUCT(AsObject)
    struct ScriptData
    {
        PROPERTY()
        std::string ScriptName;
        PROPERTY()
        std::string Name;
        PROPERTY()
        std::string Type;
        PROPERTY()
        rttr::variant Value;

        FUNCTION()
        void Serialize(ISerializer* serializer) const;

        FUNCTION()
        void Deserialize(IDeserializer* deserializer);

        ScriptData() {}

        RTTR_ENABLE()
    };
}
