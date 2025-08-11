#include "hkpch.h"
#include "AssetManager.h"
#include "Haketon/Renderer/Texture.h"

namespace Haketon
{
    std::unordered_map<UUID, Ref<Asset>> AssetManager::s_LoadedAssets;
    std::unordered_map<UUID, AssetMetadata> AssetManager::s_AssetRegistry;
    
    void AssetManager::Init()
    {
        // TODO: Load the AssetRegistry from a file or reflection system (e.g., AssetRegistry.yaml)
        HK_CORE_INFO("AssetManager Initialized");
    }

    void AssetManager::Shutdown()
    {
        s_LoadedAssets.clear();
        s_AssetRegistry.clear();
        HK_CORE_INFO("AssetManager Shutdown");
    }

    const AssetMetadata& AssetManager::GetAssetMetadata(UUID handle)
    {
        static AssetMetadata s_NullMetadata;
        auto it = s_AssetRegistry.find(handle);
        if (it == s_AssetRegistry.end())
        {
            HK_CORE_ERROR("No metadata found for asset handle: {}", handle.ToString());
            return s_NullMetadata;
        }
        return it->second;
    }

    Ref<Asset> AssetManager::GetAssetInternal(UUID handle)
    {
        if (s_LoadedAssets.find(handle) != s_LoadedAssets.end())
        {
            return s_LoadedAssets[handle];
        }

        const AssetMetadata& metadata = GetAssetMetadata(handle);
        if (!metadata.IsDataLoaded())
        {
            HK_CORE_ERROR("Asset metadata is invalid for handle: {}", handle.ToString());
            return nullptr;
        }

        Ref<Asset> asset = nullptr;
        switch (metadata.Type)
        {
            case AssetType::Texture:
            {
                asset = Texture2D::Create(metadata.FilePath.string());
                break;
            }
            case AssetType::None:
            case AssetType::Mesh:
            case AssetType::Material:
            case AssetType::Scene:
            default: HK_CORE_ERROR("Asset loading for this type not implemented yet!"); break;
        }

        if (!asset)
        {
            HK_CORE_ERROR("Failed to load asset: {}", metadata.FilePath.string());
            return nullptr;
        }

        asset->m_Handle = handle;
        s_LoadedAssets[handle] = asset;
        return asset;
    }

#ifdef HK_EDITOR
    UUID AssetManager::ImportAsset(const std::filesystem::path& sourcePath)
    {
        // This is a placeholder for a much more complex system.
        // In a real engine, this would:
        // 1. Determine the asset type from the file extension.
        // 2. Invoke the correct AssetImporter (e.g., TextureImporter).
        // 3. The importer would generate a new handle, cook the data,
        //    and create the metadata.
        // 4. We would then add the new metadata to our s_AssetRegistry.
        // 5. Finally, we would save the entire registry to disk.

        HK_CORE_WARN("Asset Importer not implemented yet!");
        return UUID::Null();
    }
#endif
}
