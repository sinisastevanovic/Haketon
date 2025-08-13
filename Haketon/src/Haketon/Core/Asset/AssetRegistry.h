#pragma once

#include "AssetMetadata.h"
#include <unordered_map>

namespace Haketon
{
    class HK_API AssetRegistry
    {
    public:
        bool LoadCache(const std::filesystem::path& cachePath);
        bool SaveCache(const std::filesystem::path& cachePath);

        void ScanAndSync(const std::filesystem::path& assetDirectory);

        const AssetMetadata* GetMetadata(UUID handle) const;
        const AssetMetadata* GetMetadata(const std::filesystem::path& sourcePath) const;
        UUID GetHandle(const std::filesystem::path& sourcePath) const;

        // Used by the editor
        void RegisterNewAsset(const AssetMetadata& metadata);

        bool LoadMetadataFromMetaFile(const std::filesystem::path& metaFilePath, AssetMetadata& metadata);

        std::vector<UUID> GetAssetsInDirectory(const std::filesystem::path& directoryPath) const;

        bool MoveAsset(UUID handle, const std::filesystem::path& newSourcePath);
    private:

        bool SaveMetadataFile(UUID handle);
        
        std::unordered_map<UUID, AssetMetadata> m_RegistryByHandle;
        std::unordered_map<std::filesystem::path, UUID> m_HandleByPath;

        friend class AssetManager;
    
    };
}


