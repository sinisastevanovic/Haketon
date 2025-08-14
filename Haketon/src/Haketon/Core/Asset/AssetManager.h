#pragma once

#include "Haketon/Core/Core.h"
#include "Asset.h"
#include "AssetRegistry.h"
#include <memory>
#include <unordered_map>

#include "Haketon/Renderer/Texture.h"

namespace Haketon
{
    class HK_API AssetManager
    {
    public:
        static void Init();
        static void Shutdown();

        static bool IsAssetLoaded(UUID handle);
        
        template<typename T>
        static Ref<T> GetAsset(UUID handle)
        {
            static_assert(std::is_base_of<Asset, T>::value, "T must derive from Asset");

            const AssetMetadata* metadata = GetMetadata(handle);
            if (!metadata)
            {
                HK_CORE_ERROR("AssetManager::GetAsset - No metadata found for handle: {0}", handle);
                return nullptr;
            }

            if (IsAssetLoaded(handle))
                return std::static_pointer_cast<T>(s_LoadedAssets.at(handle));

            return LoadAsset<T>(*metadata);
        }

        static const AssetMetadata* GetMetadata(UUID handle);
        static const AssetMetadata* GetMetadata(const std::filesystem::path& sourcePath);
        static UUID GetHandleByPath(const std::filesystem::path& sourcePath);

        static std::vector<UUID> GetAssetsInDirectory(const std::filesystem::path& directoryPath);

#ifdef HK_EDITOR
        static UUID ImportAsset(const std::filesystem::path& sourcePath);
        static bool ReloadAsset(UUID handle);

        static bool MoveAsset(UUID handle, const std::filesystem::path& destinationPath);
        static bool DeleteAsset(UUID handle);
        static bool DeleteDirectory(const std::filesystem::path& directoryPath);
        static bool CreateDir(const std::filesystem::path& directoryPath);

        static bool FoundUnimportedAssets();
        static void ImportUnimportedAssets();
#endif

    private:
        template<typename T>
        static Ref<T> LoadAsset(const AssetMetadata& metadata)
        {
            static_assert(std::is_base_of<Asset, T>::value, "T must derive from Asset");

            Ref<Asset> asset = nullptr;
            switch (metadata.Type)
            {
                case AssetType::Texture:
                    asset = Texture2D::Create(metadata.CookedFilePath);
                    break;
                default:
                    HK_CORE_ERROR("AssetManager::LoadAsset - No loader available for asset type!");
                    return nullptr;
            }

            if (!asset)
            {
                HK_CORE_ERROR("AssetManager::LoadAsset - Failed to load asset from: {}", metadata.CookedFilePath.string());
                return nullptr;
            }

            asset->m_Handle = metadata.Handle;
            s_LoadedAssets[metadata.Handle] = asset;
            return std::static_pointer_cast<T>(asset);
        }
        
        static std::unordered_map<UUID, Ref<Asset>> s_LoadedAssets;
        static std::unique_ptr<AssetRegistry> s_ActiveRegistry;
    };
}

