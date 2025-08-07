#include "hkpch.h"
#include "UUID.h"

#include "Haketon/Core/Serialization/ISerializationContext.h"

namespace Haketon
{
    std::random_device UUID::s_randomDevice;
    std::mt19937_64 UUID::s_randomEngine(UUID::s_randomDevice());
    std::uniform_int_distribution<uint64_t> UUID::s_distribution;

    UUID::UUID()
        : m_Value(s_distribution(s_randomEngine))
    {
        while (m_Value == 0)
        {
            m_Value = s_distribution(s_randomEngine);
        }
    }

    UUID::UUID(uint64_t value)
        : m_Value(value)
    {
    }

    const UUID& UUID::Null()
    {
        static const UUID nullUUID(0);
        return nullUUID;
    }

    std::string UUID::ToString() const
    {
        return fmt::format("{:x}", m_Value);
    }

    UUID UUID::FromString(const std::string& str)
    {
        try
        {
            return UUID(std::stoull(str, nullptr, 16));
        }
        catch(std::exception& e)
        {
            HK_CORE_ERROR("Error parsing UUID string {0}", str);
            return UUID::Null();
        }
    }

    void UUID::Serialize(ISerializer* serializer) const
    {
        serializer->Serialize("UUID", m_Value);
    }

    void UUID::Deserialize(IDeserializer* deserializer)
    {
        deserializer->Deserialize("UUID", m_Value);
    }
}
