#pragma once

#include "Haketon/Core/Core.h"
#include "AssetMetadata.h"

namespace Haketon
{
    class AssetImporter
    {
    public:
        virtual ~AssetImporter() = default;

        virtual bool Import(const std::filesystem::path& sourcePath, AssetMetadata& outMetadata) = 0;
    };

    class AssetImporterFactory
    {
    public:
        static std::unique_ptr<AssetImporter> Create(AssetType type);
    };
}