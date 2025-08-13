#include "TextureImporter.h"

namespace Haketon
{
    bool TextureImporter::Import(const std::filesystem::path& sourcePath, AssetMetadata& outMetadata)
    {
        HK_CORE_INFO("Importing texture from: {}", sourcePath.string());
        return true;
    }
}

