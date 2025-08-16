#pragma once

#include "Haketon/Core/Misc/UUID.h"
#include "Haketon/Core/IReflectable.h"

namespace Haketon
{
    CLASS()
    class HK_API AssetHandle
    {
    public:
        AssetHandle() : m_UUID() {}
        explicit AssetHandle(const UUID& uuid) : m_UUID(uuid) {}
        explicit AssetHandle(uint64_t value) : m_UUID(value) {}

        static const AssetHandle& Null() 
        { 
            static AssetHandle nullHandle(UUID::Null()); 
            return nullHandle; 
        }

        bool IsValid() const { return m_UUID.IsValid(); }
        uint64_t GetValue() const { return m_UUID.GetValue(); }
        std::string ToString() const { return m_UUID.ToString(); }

        // Conversion operators
        operator UUID() const { return m_UUID; }
        const UUID& GetUUID() const { return m_UUID; }

        // Operators
        bool operator==(const AssetHandle& other) const { return m_UUID == other.m_UUID; }
        bool operator!=(const AssetHandle& other) const { return m_UUID != other.m_UUID; }
        bool operator<(const AssetHandle& other) const { return m_UUID < other.m_UUID; }

        // Comparison with UUID
        bool operator==(const UUID& uuid) const { return m_UUID == uuid; }
        bool operator!=(const UUID& uuid) const { return m_UUID != uuid; }

        explicit operator uint64_t() const { return static_cast<uint64_t>(m_UUID); }

        FUNCTION()
        void Serialize(ISerializer* serializer) const { m_UUID.Serialize(serializer); }

        FUNCTION()
        void Deserialize(IDeserializer* deserializer) { m_UUID.Deserialize(deserializer); }

    private:
        UUID m_UUID;
    };
}

// Hashing for std::unordered_map
namespace std
{
    template <>
    struct hash<Haketon::AssetHandle>
    {
        size_t operator()(const Haketon::AssetHandle& handle) const noexcept
        {
            return std::hash<Haketon::UUID>()(handle.GetUUID());
        }
    };
}

inline std::ostream& operator<<(std::ostream& os, const Haketon::AssetHandle& handle)
{
    return os << handle.ToString();
}

template<>
struct fmt::formatter<Haketon::AssetHandle>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}')
        {
            throw format_error("invalid format");
        }
        return it;
    }

    template <typename FormatContext>
    auto format(const Haketon::AssetHandle& handle, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", handle.ToString());
    }
};