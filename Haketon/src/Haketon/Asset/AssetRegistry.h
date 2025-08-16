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

        const AssetMetadata* GetMetadata(AssetHandle handle) const;
        const AssetMetadata* GetMetadata(const std::filesystem::path& sourcePath) const;
        AssetHandle GetHandle(const std::filesystem::path& sourcePath) const;

        // Used by the editor
        void RegisterNewAsset(const AssetMetadata& metadata);

        bool LoadMetadataFromMetaFile(const std::filesystem::path& metaFilePath, AssetMetadata& metadata);

        std::vector<AssetHandle> GetAssetsInDirectory(const std::filesystem::path& directoryPath) const;
        std::vector<AssetMetadata> GetAllAssetsOfType(AssetType type) const;
        std::vector<AssetMetadata> GetAllAssetsOfTypeSorted(AssetType type) const;

        bool MoveAsset(AssetHandle handle, const std::filesystem::path& newSourcePath);
        void RemoveAsset(AssetHandle handle);

        bool FoundUnimportedAssets() const { return !m_UnimportedAssets.empty(); }
        const std::vector<std::filesystem::path>& GetUnimportedAssets() const { return m_UnimportedAssets; }
        void ClearUnimportedAssets() { m_UnimportedAssets.clear(); }
    private:

        bool SaveMetadataFile(AssetHandle handle);
        
        std::unordered_map<AssetHandle, AssetMetadata> m_RegistryByHandle;
        std::unordered_map<std::filesystem::path, AssetHandle> m_HandleByPath;
        std::vector<std::filesystem::path> m_UnimportedAssets;

        friend class AssetManager;
    
    };
}


