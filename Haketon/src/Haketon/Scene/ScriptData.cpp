#include "hkpch.h"
#include "ScriptData.h"

void Haketon::ScriptData::Serialize(ISerializer* serializer) const
{
    serializer->Serialize("ScriptName", ScriptName);
    serializer->Serialize("Name", Name);
    serializer->Serialize("Type", Type);
    serializer->SerializeValue("Value", Value);
}

void Haketon::ScriptData::Deserialize(IDeserializer* deserializer)
{
    deserializer->Deserialize("ScriptName", ScriptName);
    deserializer->Deserialize("Name", Name);
    deserializer->Deserialize("Type", Type);
    auto type = rttr::type::get_by_name(Type);
    if (type.is_valid())
    {
        Value = deserializer->CreateDefaultVariant(type);
        deserializer->DeserializeValue("Value", Value);
    }
}
