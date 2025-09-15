#include "hkpch.h"
#include "AssetManager.h"

#include "AssetImporter.h"
#include "AssetImporter/TextureImporter.h"
#include "Haketon/Core/PathUtils.h"
#include "Haketon/Core/Serialization/RapidJsonSerializer.h"
#include "Haketon/Core/Serialization/TypeHandlerRegistry.h"
#include "Haketon/Renderer/Texture.h"

namespace Haketon
{
    std::unordered_map<AssetHandle, std::weak_ptr<Asset>> AssetManager::s_LoadedAssets;
    std::unordered_map<AssetHandle, std::weak_ptr<Asset>> AssetManager::s_TransientAssets;
    std::unique_ptr<AssetRegistry> AssetManager::s_ActiveRegistry;

    void AssetManager::Init()
    {
        // TODO: Add these to HeaderTool!
		TypeHandlerRegistry::GetInstance().RegisterAssetLoader<Shader>();
		TypeHandlerRegistry::GetInstance().RegisterAssetLoader<Material>();
		TypeHandlerRegistry::GetInstance().RegisterAssetLoader<Texture2D>();
		TypeHandlerRegistry::GetInstance().RegisterAssetLoader<Scene>();
        
        s_ActiveRegistry = std::make_unique<AssetRegistry>();
        
#ifdef HK_EDITOR
        std::filesystem::path cachePath = PathUtils::GetGameCachePath() / "AssetCache.bin";
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
        s_ActiveRegistry->SaveCache(PathUtils::GetGameCachePath()  / "AssetCache.bin");
#endif

        s_TransientAssets.clear();
        s_LoadedAssets.clear();
        s_ActiveRegistry.reset();
    }

    bool AssetManager::IsAssetLoaded(AssetHandle handle)
    {
        return s_LoadedAssets.find(handle) != s_LoadedAssets.end();
    }

    const AssetMetadata* AssetManager::GetMetadata(AssetHandle handle)
    {
        return s_ActiveRegistry->GetMetadata(handle);
    }

    const AssetMetadata* AssetManager::GetMetadata(const std::filesystem::path& sourcePath)
    {
        return s_ActiveRegistry->GetMetadata(sourcePath);
    }

    AssetHandle AssetManager::GetHandleByPath(const std::filesystem::path& sourcePath)
    {
        return s_ActiveRegistry->GetHandle(sourcePath);
    }

    std::vector<AssetHandle> AssetManager::GetAssetsInDirectory(const std::filesystem::path& directoryPath)
    {
        return s_ActiveRegistry->GetAssetsInDirectory(PathUtils::GetPathRelativeToAssetsPath(directoryPath));
    }

    std::vector<AssetMetadata> AssetManager::GetAllAssetsOfType(AssetType type)
    {
        return s_ActiveRegistry->GetAllAssetsOfType(type);
    }

    std::vector<AssetMetadata> AssetManager::GetAllAssetsOfTypeSorted(AssetType type)
    {
        return s_ActiveRegistry->GetAllAssetsOfTypeSorted(type);
    }



#ifdef HK_EDITOR
    
    bool AssetManager::SaveMetadataFile(AssetMetadata& metadata)
    {
        std::filesystem::path newMetaPath = std::string(GetFileSystemPath(metadata.SourceFilePath).string()) + ".meta";

        auto currentTime = std::filesystem::file_time_type::clock::now();
        metadata.MetaFileTimestamp = AssetMetadata::FileTimestampToInt(currentTime);
        RapidJsonSerializer rs;
        rs.SerializeObject(metadata);
        rs.SaveToFile(newMetaPath);
        return true;
    }
    
    std::unique_ptr<AssetImporter> AssetImporterFactory::Create(AssetType type)
    {
        switch (type)
        {
            case AssetType::Texture: return std::make_unique<TextureImporter>();
        }

        return nullptr;
    }
    
    AssetHandle AssetManager::ImportAsset(const std::filesystem::path& sourcePath)
    {
        HK_CORE_INFO("Importing asset from {}", sourcePath.string());
        if (!std::filesystem::exists(sourcePath))
        {
            HK_CORE_ERROR("AssetManager::ImportAsset - Source file does not exist: {}", sourcePath.string());
            return AssetHandle::Null();
        }

        AssetType newAssetType = AssetUtils::GetAssetTypeFromExtension(sourcePath);
        if (newAssetType == AssetType::None)
        {
            HK_CORE_ERROR("AssetManager::ImportAsset - Asset type not supported.");
            return AssetHandle::Null();
        }

        std::filesystem::path srcPath = sourcePath;

        std::filesystem::path gameAssetPath = PathUtils::GetGameAssetsPath();
        auto rel = std::filesystem::relative(sourcePath, gameAssetPath);
        if (rel.empty() || rel.native()[0] == '.')
        {
            srcPath = gameAssetPath / sourcePath.filename();
            // This asset is not in our asset directory. So copy it into it.
            if (!std::filesystem::copy_file(sourcePath, srcPath))
            {
                HK_CORE_ERROR("Could not copy file into asset directoy: {0} to {1}", sourcePath.string(), srcPath.string());
                return AssetHandle::Null();
            }
        }

        std::filesystem::path metaPath = srcPath;
        metaPath += ".meta";

        AssetMetadata metadata;
        bool isNewAsset = !std::filesystem::exists(metaPath);

        if (isNewAsset)
        {
            metadata.Handle = AssetHandle();
            metadata.SourceFilePath = PathUtils::GetPathRelativeToAssetsPath(sourcePath);
            metadata.Type = newAssetType;
            auto sourceTimestamp = std::filesystem::last_write_time(srcPath);
            metadata.SourceFileTimestamp = AssetMetadata::FileTimestampToInt(sourceTimestamp);
            
            if (metadata.Type == AssetType::None)
            {
                HK_CORE_WARN("AssetManager::ImportAsset - Unknown asset type for file: {}", srcPath.string());
                return AssetHandle::Null();
            }
        }
        else
        {
            if (!s_ActiveRegistry->LoadMetadataFromMetaFile(metaPath, metadata))
            {
                HK_CORE_ERROR("AssetManager::ImportAsset - Failed to load existing .meta file: {}", metaPath.string());
                return AssetHandle::Null();
            }
        }

        std::unique_ptr<AssetImporter> importer = AssetImporterFactory::Create(newAssetType);
        if (!importer)
        {
            HK_CORE_ERROR("No importer available for asset type! Skipping cooking!");
            //return AssetHandle::Null();
        }

        if (importer && !importer->Import(sourcePath, metadata))
        {
            HK_CORE_ERROR("Failed to cook asset: {}", sourcePath.string());
            return AssetHandle::Null();
        }

        SaveMetadataFile(metadata);
        s_ActiveRegistry->RegisterNewAsset(metadata);
        

        HK_CORE_INFO("Asset imported successfully!");
        return metadata.Handle;
    }

    bool AssetManager::ReloadAsset(AssetHandle handle)
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

    bool AssetManager::MoveAsset(AssetHandle handle, const std::filesystem::path& destinationPath)
    {
        // TODO: We need to check if a file with the same name already exists at the destination!!!
        AssetMetadata* metadata = s_ActiveRegistry->GetMetadata(handle);
        if (!metadata)
        {
            HK_ERROR("AssetManager::MoveAsset - No metadata for handle: {}", handle);
            return false;
        }

        std::filesystem::path oldSourcePath = GetFileSystemPath(metadata->SourceFilePath);
        std::filesystem::path oldMetaPath = std::string(oldSourcePath.string()) + ".meta";

        std::filesystem::path newSourcePath = destinationPath / oldSourcePath.filename();
        std::filesystem::path newMetaPath = std::string(newSourcePath.string()) + ".meta";

        try
        {
            std::filesystem::rename(oldSourcePath, newSourcePath);
            std::filesystem::rename(oldMetaPath, newMetaPath);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            HK_CORE_ERROR("AssetManager::MoveAsset - Filesystem error: {}", e.what());
            return false;
        }

        s_ActiveRegistry->MoveAsset(handle, PathUtils::GetPathRelativeToAssetsPath(newSourcePath));
        SaveMetadataFile(*(s_ActiveRegistry->GetMetadata(handle)));

        HK_CORE_INFO("Moved asset '{}' to '{}'", oldSourcePath.string(), newSourcePath.string());
        return true;
    }

    bool AssetManager::DeleteAsset(AssetHandle handle)
    {
        const AssetMetadata* metadata = GetMetadata(handle);
        if (!metadata)
            return false;

        try
        {
            std::filesystem::remove(GetFileSystemPath(metadata->SourceFilePath));
            std::filesystem::remove(std::string(GetFileSystemPath(metadata->SourceFilePath).string()) + ".meta");
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            HK_CORE_ERROR("AssetManager::DeleteAsset - Filesystem error: {}", e.what());
            return false;
        }

        s_ActiveRegistry->RemoveAsset(handle);
        return true;
    }

    bool AssetManager::DeleteDirectory(const std::filesystem::path& directoryPath)
    {
        try
        {
            for (auto& p : std::filesystem::recursive_directory_iterator(directoryPath))
            {
                if (p.is_directory())
                    continue;
                
                const AssetMetadata* metadata = GetMetadata(p);
                if (metadata)
                {
                    DeleteAsset(metadata->Handle);
                }
            }

            std::filesystem::remove(directoryPath);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            HK_CORE_ERROR("AssetManager::DeleteDirectory - Filesystem error: {}", e.what());
            return false;
        }

        return true;
    }

    bool AssetManager::CreateDir(const std::filesystem::path& directoryPath)
    {
        bool success = false;
        try
        {
            success =  std::filesystem::create_directory(directoryPath);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            HK_CORE_ERROR("AssetManager::CreateDir - Filesystem error: {}", e.what());
            return false;
        }

        return success;
    }

    bool AssetManager::FoundUnimportedAssets()
    {
        return s_ActiveRegistry->FoundUnimportedAssets();
    }

    void AssetManager::ImportUnimportedAssets()
    {
        for (auto path : s_ActiveRegistry->GetUnimportedAssets())
        {
            ImportAsset(path);
        }

        s_ActiveRegistry->ClearUnimportedAssets();
    }
#endif
}
