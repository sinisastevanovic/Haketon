#pragma once

#include "Asset.h"
#include <filesystem>

namespace Haketon
{
    STRUCT()
    struct AssetMetadata
    {
        AssetMetadata() = default;

        PROPERTY()
        UUID Handle = UUID::Null();

        PROPERTY()
        AssetType Type = AssetType::None;

        PROPERTY()
        std::filesystem::path SourceFilePath;
        PROPERTY()
        std::filesystem::path CookedFilePath;

        PROPERTY()
        int64_t SourceFileTimestamp;
        PROPERTY()
        int64_t MetaFileTimestamp;

        bool IsDataLoaded() const { return Handle.IsValid(); }

        void Serialize(std::ostream& out) const;
        void Deserialize(std::istream& in);

        void Serialize(ISerializer* serializer) const;
        void Deserialize(IDeserializer* deserializer);

        static int64_t FileTimestampToInt(std::filesystem::file_time_type fileTimestamp)
        {
            const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(fileTimestamp);
            return std::chrono::system_clock::to_time_t(systemTime);
        }
    };
}
