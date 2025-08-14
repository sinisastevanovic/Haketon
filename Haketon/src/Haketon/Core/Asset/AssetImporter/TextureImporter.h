#pragma once

#include "Haketon/Core/Asset/AssetImporter.h"

namespace Haketon
{
    class TextureImporter : public AssetImporter
    {
    public:
        virtual bool Import(const std::filesystem::path& sourcePath, AssetMetadata& outMetadata) override;
    
    };
}

