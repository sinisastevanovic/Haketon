#pragma once

#include "Haketon/Core/Core.h"
#include <filesystem>

namespace Haketon
{
    ENUM()
    enum class AssetType : uint16_t
    {
        None = 0,
        Texture,
        Mesh,
        Material,
        Shader,
        Scene
    };
    
    namespace AssetUtils
    {
        HK_API const char* GetFilterForSupportedAssets();
        HK_API const char* GetFilterForAssetType(AssetType type);
        HK_API AssetType GetAssetTypeFromExtension(const std::filesystem::path& path);
        HK_API bool IsAssetExtensionSupported(const std::filesystem::path& path);
    }
}