#include "hkpch.h"
#include "AssetManager.h"

#include "AssetImporter.h"
#include "Haketon/Core/PathUtils.h"
#include "Haketon/Core/Serialization/RapidJsonSerializer.h"
#include "Haketon/Renderer/Texture.h"

namespace Haketon
{
    std::unordered_map<UUID, Ref<Asset>> AssetManager::s_LoadedAssets;
    std::unique_ptr<AssetRegistry> AssetManager::s_ActiveRegistry;

    void AssetManager::Init()
    {
        s_ActiveRegistry = std::make_unique<AssetRegistry>();
        
#ifdef HK_EDITOR
        std::filesystem::path cachePath = PathUtils::GetGameTmpPath() / "AssetCache.bin";
        if (s_ActiveRegistry->LoadCache(cachePath))
        {
            s_ActiveRegistry->ScanAndSync(PathUtils::GetGameAssetsPath());
        } 
        else
        {  
            s_ActiveRegistry->ScanAndSync(PathUtils::GetGameAssetsPath());
        }
#else
        s_ActiveRegistry->LoadCache("AssetRegistry.bin"); // TODO: Use PathUtils
#endif
    }

    void AssetManager::Shutdown()
    {
#ifdef HK_EDITOR
        s_ActiveRegistry->SaveCache(PathUtils::GetGameTmpPath() / "AssetCache.bin");
#endif

        s_LoadedAssets.clear();
        s_ActiveRegistry.reset();
    }

    bool AssetManager::IsAssetLoaded(UUID handle)
    {
        return s_LoadedAssets.find(handle) != s_LoadedAssets.end();
    }

    const AssetMetadata* AssetManager::GetMetadata(UUID handle)
    {
        return s_ActiveRegistry->GetMetadata(handle);
    }

    const AssetMetadata* AssetManager::GetMetadata(const std::filesystem::path& sourcePath)
    {
        return s_ActiveRegistry->GetMetadata(sourcePath);
    }

    UUID AssetManager::GetHandleByPath(const std::filesystem::path& sourcePath)
    {
        return s_ActiveRegistry->GetHandle(sourcePath);
    }

    std::vector<UUID> AssetManager::GetAssetsInDirectory(const std::filesystem::path& directoryPath)
    {
        return s_ActiveRegistry->GetAssetsInDirectory(directoryPath);
    }

#ifdef HK_EDITOR
    static AssetType GetTypeFromExtension(const std::string& extension)
    {
        if (extension == ".png" || extension == ".jpg" || extension == ".jpeg")
            return AssetType::Texture;
        return AssetType::None;
    }
    
    UUID AssetManager::ImportAsset(const std::filesystem::path& sourcePath)
    {
        if (!std::filesystem::exists(sourcePath))
        {
            HK_CORE_ERROR("AssetManager::ImportAsset - Source file does not exist: {}", sourcePath.string());
            return UUID::Null();
        }

        std::filesystem::path metaPath = sourcePath;
        metaPath += ".meta";

        AssetMetadata metadata;
        bool isNewAsset = !std::filesystem::exists(metaPath);

        if (isNewAsset)
        {
            metadata.Handle = UUID();
            metadata.SourceFilePath = sourcePath;
            metadata.Type = GetTypeFromExtension(sourcePath.extension().string());
            auto sourceTimestamp = std::filesystem::last_write_time(sourcePath);
            metadata.SourceFileTimestamp = AssetMetadata::FileTimestampToInt(sourceTimestamp);
            
            if (metadata.Type == AssetType::None)
            {
                HK_CORE_WARN("AssetManager::ImportAsset - Unknown asset type for file: {}", sourcePath.string());
                return UUID::Null();
            }
        }
        else
        {
            if (!s_ActiveRegistry->LoadMetadataFromMetaFile(metaPath, metadata))
            {
                HK_CORE_ERROR("AssetManager::ImportAsset - Failed to load existing .meta file: {}", metaPath.string());
                return UUID::Null();
            }
        }

        std::unique_ptr<AssetImporter> importer;
        // TODO: we need a registry of asset importers... Or just place all of them in the core engine?  
        /*switch (metadata.Type)
        {
        }*/


        RapidJsonSerializer rs;
        rs.SerializeObject(metadata);
        rs.SaveToFile(metaPath);
        auto metaTimestamp = std::filesystem::last_write_time(metaPath);
        metadata.MetaFileTimestamp = AssetMetadata::FileTimestampToInt(metaTimestamp);
        s_ActiveRegistry->RegisterNewAsset(metadata);
        return metadata.Handle;
    }

    bool AssetManager::ReloadAsset(UUID handle)
    {
        if (!IsAssetLoaded(handle))
        {
            HK_CORE_WARN("AssetManager::ReloadAsset - Asset not loaded, cannot reload: {}", handle);
            return false;
        }

        const AssetMetadata* metadata = GetMetadata(handle);
        if (!metadata)
        {
            HK_CORE_ERROR("AssetManager::ReloadAsset - No metadata for handle: {}", handle);
            return false;
        }

        // Re-run the import process to update the cooked asset file on disk.
        //if (!AssetImporter::Import(metadata->SourceFilePath, *metadata))
        if (true)
        {
            HK_CORE_ERROR("Failed to re-import asset for reload: {}", metadata->SourceFilePath.string());
            return false;
        }

        // The asset is already loaded, so we need to replace its data.
        // A real implementation would have a `LoadFromData` method on the Asset.
        // For now, we'll just remove and re-load it.
        s_LoadedAssets.erase(handle);
        GetAsset<Asset>(handle); // This will trigger a load from the newly cooked file.

        HK_CORE_INFO("Reloaded asset: {}", handle);
        return true;
    }
#endif
}
