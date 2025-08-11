#pragma once

#include "Haketon/Core/Core.h"
#include "Asset.h"
#include "AssetMetadata.h"
#include <memory>
#include <unordered_map>

namespace Haketon
{
    class AssetManager
    {
    public:
        static void Init();
        static void Shutdown();

        template<typename T>
        static Ref<T> GetAsset(UUID handle)
        {
            static_assert(std::is_base_of<Asset, T>::value, "T must derive from Asset");

            Ref<Asset> asset = GetAssetInternal(handle);
            return std::static_pointer_cast<T>(asset);
        }

        static const AssetMetadata& GetAssetMetadata(UUID handle);

#ifdef HK_EDITOR
        static UUID ImportAsset(const std::filesystem::path& sourcePath);
#endif

    private:
        static Ref<Asset> GetAssetInternal(UUID handle);
        
        static std::unordered_map<UUID, Ref<Asset>> s_LoadedAssets;

        static std::unordered_map<UUID, AssetMetadata> s_AssetRegistry;
    };
}

