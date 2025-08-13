#pragma once

#include "Haketon/Core/Core.h"
#include "AssetMetadata.h"

namespace Haketon
{
    class HK_API AssetImporter
    {
    public:
        virtual ~AssetImporter() = default;

        virtual bool Import(const std::filesystem::path& sourcePath, AssetMetadata& outMetadata) = 0;
    };
}