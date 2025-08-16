#pragma once

#include "Haketon/Core/Core.h"

#include <rttr/type>
#include <random>

#include "Haketon/Core/IReflectable.h"
#include "Haketon/Core/Serialization/ISerializationContext.h"


namespace Haketon
{
    // TODO: Add class to header tool!
    CLASS()
    class HK_API UUID
    {
    public:
        // Default constructor: Generates a new UUID
        UUID();

        // TODO: We need to handle more than default constructor in Header Tool
    
        // Constructor from an existing 64-bit value (e.g., for deserialization)
        explicit UUID(uint64_t value);

        // Creates a invalid UUID (all zeroes)
        static const UUID& Null();

        // Check if the UUID is Valid
        bool IsValid() const { return m_Value != 0; }

        // Get the raw 64-bit value
        uint64_t GetValue() const { return m_Value; }

        // Convert UUID to a string representation (e.g., hex)
        std::string ToString() const;
    
        // Create a UUID from a string
        static UUID FromString(const std::string& str);

        FUNCTION()
        void Serialize(ISerializer* serializer) const;

        FUNCTION()
        void Deserialize(IDeserializer* deserializer);

        bool operator==(const UUID& other) const { return m_Value == other.m_Value; }
        bool operator!=(const UUID& other) const { return !(*this == other); }
        bool operator<(const UUID& other) const { return m_Value < other.m_Value; }
        explicit operator uint64_t() const { return m_Value; }

    private:
        uint64_t m_Value;

        static std::random_device s_randomDevice;
        static std::mt19937_64 s_randomEngine;
        static std::uniform_int_distribution<uint64_t> s_distribution;

        RTTR_ENABLE()
    };
}

// --- Hashing for std::unordered_map ---
// This specialization allows UUID to be used as a key in std::unordered_map
namespace std
{
    template <>
    struct hash<Haketon::UUID>
    {
        size_t operator()(const Haketon::UUID& uuid) const noexcept
        {
            return std::hash<uint64_t>()(uuid.GetValue());
        }
    };
}

inline std::ostream& operator<<(std::ostream& os, const Haketon::UUID& uuid)
{
    return os << uuid.ToString();
}

template<>
struct fmt::formatter<Haketon::UUID>
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
    auto format(const Haketon::UUID& uuid, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", uuid.ToString());
    }
};