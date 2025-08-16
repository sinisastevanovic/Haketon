#pragma once

#include "Haketon/Core/Core.h"
#include "Asset.h"
#include "AssetRegistry.h"
#include <memory>
#include <unordered_map>
#include <rttr/registration.h>

#include "Haketon/Core/PathUtils.h"
#include "Haketon/Renderer/Texture.h"
#include "Haketon/Scene/Scene.h"

namespace Haketon
{
    class HK_API AssetManager
    {
    public:
        static void Init();
        static void Shutdown();

        static bool IsAssetLoaded(AssetHandle handle);

        static const AssetMetadata* GetMetadata(AssetHandle handle);
        static const AssetMetadata* GetMetadata(const std::filesystem::path& sourcePath);
        static const AssetMetadata* GetMetadata(const Ref<Asset>& asset) { return GetMetadata(asset->m_Handle); }

        static std::filesystem::path GetFileSystemPath(const std::filesystem::path& relativePath) { return PathUtils::GetGameAssetsPath() / relativePath; }
        static std::string GetFileSystemPathString(const std::filesystem::path& relativePath) { return GetFileSystemPath(relativePath).string(); }
        
        static AssetHandle GetHandleByPath(const std::filesystem::path& sourcePath);
        static bool IsAssetHandleValid(AssetHandle handle) { return IsTransientAsset(handle) || GetMetadata(handle); }
        static bool IsTransientAsset(AssetHandle handle) { return s_TransientAssets.find(handle) != s_TransientAssets.end(); }

        static const std::unordered_map<AssetHandle, Ref<Asset>>& GetLoadedAssets() { return s_LoadedAssets; }
        static const std::unordered_map<AssetHandle, Ref<Asset>>& GetTransientAssets() { return s_TransientAssets; }
        
        template<typename T>
        static Ref<T> GetAsset(AssetHandle handle)
        {
            static_assert(std::is_base_of<Asset, T>::value, "T must derive from Asset");

            if (IsTransientAsset(handle))
            {
                return std::static_pointer_cast<T>(s_TransientAssets.at(handle));
            }

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

        template<typename T>
        static Ref<T> CreateTransientAsset()
        {
            static_assert(std::is_base_of<Asset, T>::value, "T must derive from Asset");

            Ref<Asset> asset = CreateRef<T>();
            asset->m_Handle = AssetHandle();
            s_TransientAssets[asset->m_Handle] = asset;
            return std::static_pointer_cast<T>(asset);
        }

        static uint32_t GetNumTransientAssets() { return s_TransientAssets.size(); }

        static std::vector<AssetHandle> GetAssetsInDirectory(const std::filesystem::path& directoryPath);
        static std::vector<AssetMetadata> GetAllAssetsOfType(AssetType type);
        static std::vector<AssetMetadata> GetAllAssetsOfTypeSorted(AssetType type);

#ifdef HK_EDITOR
        static bool SaveMetadataFile(AssetMetadata& metadata);
        
        static AssetHandle ImportAsset(const std::filesystem::path& sourcePath);
        static bool ReloadAsset(AssetHandle handle);

        static bool MoveAsset(AssetHandle handle, const std::filesystem::path& destinationPath);
        static bool DeleteAsset(AssetHandle handle);
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

            // TODO: We should generalize this more.
            // These create methods are not it. We should probably do some Factory stuff or so.
            Ref<Asset> asset = nullptr;
            switch (metadata.Type)
            {
                case AssetType::Texture:
                    asset = Texture2D::Create(GetFileSystemPath(metadata.CookedFilePath));
                    break;
                case AssetType::Scene:
                    asset = Scene::Create(GetFileSystemPath(metadata.SourceFilePath)); // TODO: Cooking not supported for scenes yet!
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
            asset->m_Name = metadata.SourceFilePath.stem().string();
            asset->m_Path = metadata.SourceFilePath.string();
            s_LoadedAssets[metadata.Handle] = asset;
            return std::static_pointer_cast<T>(asset);
        }

        // TODO: We need to think about ownership here. If we keep storing shared_ptrs, Assets never get unloaded.
        // But isn't it natural that the AssetManager owns the Assets? I don't know...
        static std::unordered_map<AssetHandle, Ref<Asset>> s_LoadedAssets; // TODO: Do assets ever get unloaded?
        static std::unordered_map<AssetHandle, Ref<Asset>> s_TransientAssets; // TODO: Do assets ever get unloaded?
        static std::unique_ptr<AssetRegistry> s_ActiveRegistry;
    };
}

