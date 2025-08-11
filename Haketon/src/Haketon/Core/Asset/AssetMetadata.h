#pragma once

#include "Asset.h"
#include <filesystem>

namespace Haketon
{
    struct AssetMetadata
    {
        UUID Handle = UUID::Null();
        AssetType Type = AssetType::None;
        std::filesystem::path FilePath;

        bool IsDataLoaded() const { return Handle.IsValid(); }
    };
}
