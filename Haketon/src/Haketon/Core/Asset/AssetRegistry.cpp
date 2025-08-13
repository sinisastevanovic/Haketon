#include "hkpch.h"
#include "AssetRegistry.h"

#include <fstream>
#include <rttr/registration.h>

#include "Haketon/Core/Serialization/RapidJsonDeserializer.h"
#include "Haketon/Core/Serialization/RapidJsonSerializer.h"

namespace Haketon
{
    bool AssetRegistry::LoadCache(const std::filesystem::path& cachePath)
    {
        if (!std::filesystem::exists(cachePath))
            return false;

        std::ifstream in(cachePath, std::ios::binary);
        if (!in.is_open())
        {
            HK_CORE_ERROR("Failed to open asset cache {0}", cachePath.string());
            return false;
        }

        size_t count = 0;
        in.read(reinterpret_cast<char*>(&count), sizeof(size_t));

        for (size_t i = 0; i < count; ++i)
        {
            AssetMetadata metadata;
            metadata.Deserialize(in);
            m_RegistryByHandle[metadata.Handle] = metadata;
            m_HandleByPath[metadata.SourceFilePath] = metadata.Handle;
        }

        HK_CORE_INFO("Loaded {0} assets from cache.", count);
        return true;
    }

    bool AssetRegistry::SaveCache(const std::filesystem::path& cachePath)
    {
        std::ofstream out(cachePath, std::ios::binary);
        if (!out.is_open())
        {
            HK_CORE_ERROR("Failed to save asset cache {0}", cachePath.string());
            return false;
        }

        size_t count = m_RegistryByHandle.size();
        out.write(reinterpret_cast<char*>(&count), sizeof(size_t));

        for (const auto& [handle, metadata] : m_RegistryByHandle)
        {
            metadata.Serialize(out);
        }

        HK_CORE_INFO("Saved {0} assets to cache.", count);
        return true;
    }

    void AssetRegistry::ScanAndSync(const std::filesystem::path& assetDirectory)
    {
        std::unordered_set<std::filesystem::path> filesOnDisk;

        for (auto& p : std::filesystem::recursive_directory_iterator(assetDirectory))
        {
            if (p.is_directory() || p.path().extension() == ".meta")
                continue;

            std::filesystem::path sourcePath = p.path();
            filesOnDisk.insert(sourcePath);

            std::filesystem::path metaPath = sourcePath;
            metaPath += ".meta";

            if (!std::filesystem::exists(metaPath))
            {
                // New asset
                HK_CORE_INFO("New unimported asset found: {}", sourcePath.string());
                // TODO: In a real editor, we should add this to a queue of assets to be re-imported.
                //AssetImporter::Import(sourcePath);
                continue;
            }

            const AssetMetadata* cachedMetadata = GetMetadata(sourcePath);
            if (!cachedMetadata)
            {
                // New asset not in cache
                HK_CORE_INFO("Discovered new asset: {0}", sourcePath.string());
                AssetMetadata newMetadata;
                if (LoadMetadataFromMetaFile(metaPath, newMetadata))
                {
                    RegisterNewAsset(newMetadata);
                    // TODO: Should probably flag for re-cooking
                }
                continue;
            }
            
            auto sourceTimestamp = std::filesystem::last_write_time(sourcePath);
            auto sourceTimestampInt = AssetMetadata::FileTimestampToInt(sourceTimestamp);
            auto metaTimestamp = std::filesystem::last_write_time(p.path());
            auto metaTimestampInt = AssetMetadata::FileTimestampToInt(metaTimestamp);
            
            // Existing asset
            if (sourceTimestampInt > cachedMetadata->SourceFileTimestamp || metaTimestampInt > cachedMetadata->MetaFileTimestamp)
            {
                HK_CORE_INFO("Asset modified, needs re-import: {0}", sourcePath.string());
                // TODO: In a real editor, we should add this to a queue of assets to be re-imported.
                //AssetImporter::Import(sourcePath);
            }
        }

        // Deleted asset
        for (auto it = m_HandleByPath.begin(); it != m_HandleByPath.end(); )
        {
            const std::filesystem::path& sourcePath = it->first;
            if (filesOnDisk.find(sourcePath) == filesOnDisk.end())
            {
                HK_CORE_INFO("Detected deleted asset: {}", sourcePath.string());
                m_RegistryByHandle.erase(it->second);
                it = m_HandleByPath.erase(it); // Erase and advance iterator
            }
            else
            {
                ++it;
            }
        }
    }

    const AssetMetadata* AssetRegistry::GetMetadata(UUID handle) const
    {
        auto it = m_RegistryByHandle.find(handle);
        return (it != m_RegistryByHandle.end()) ? &it->second : nullptr;
    }

    const AssetMetadata* AssetRegistry::GetMetadata(const std::filesystem::path& sourcePath) const
    {
        auto it= m_HandleByPath.find(sourcePath);
        if (it == m_HandleByPath.end())
            return nullptr;

        return GetMetadata(it->second);
    }

    UUID AssetRegistry::GetHandle(const std::filesystem::path& sourcePath) const
    {
        auto it = m_HandleByPath.find(sourcePath);
        return (it != m_HandleByPath.end()) ? it->second : UUID::Null();
    }

    void AssetRegistry::RegisterNewAsset(const AssetMetadata& metadata)
    {
        m_RegistryByHandle[metadata.Handle] = metadata;
        m_HandleByPath[metadata.SourceFilePath] = metadata.Handle;
    }

    bool AssetRegistry::LoadMetadataFromMetaFile(const std::filesystem::path& metaFilePath, AssetMetadata& metadata)
    {
        RapidJsonDeserializer rd;
        rd.ParseFile(metaFilePath.string());
        rttr::variant var(&metadata);
        if (rd.DeserializeObject(var))
        {
            return true;
        }

        return false;
    }

    std::vector<UUID> AssetRegistry::GetAssetsInDirectory(const std::filesystem::path& directoryPath) const
    {
        std::vector<UUID> assets;
        for (const auto& [path, handle] : m_HandleByPath)
        {
            if (path.parent_path() == directoryPath)
            {
                assets.push_back(handle);
            }
        }

        return assets;
    }

    bool AssetRegistry::MoveAsset(UUID handle, const std::filesystem::path& newSourcePath)
    {
        const std::filesystem::path& oldPath = m_RegistryByHandle.at(handle).SourceFilePath;
        
        m_HandleByPath.erase(oldPath);
        m_HandleByPath[newSourcePath] = handle;
        m_RegistryByHandle.at(handle).SourceFilePath = newSourcePath;

        return true;
    }

    bool AssetRegistry::SaveMetadataFile(UUID handle)
    {
        AssetMetadata& metadata = m_RegistryByHandle.at(handle);
        std::filesystem::path newMetaPath = std::string(metadata.SourceFilePath.string()) + ".meta";

        auto currentTime = std::filesystem::file_time_type::clock::now();
        m_RegistryByHandle.at(handle).MetaFileTimestamp = AssetMetadata::FileTimestampToInt(currentTime);
        RapidJsonSerializer rs;
        rs.SerializeObject(metadata);
        rs.SaveToFile(newMetaPath);
        return true;
    }

    static void WritePath(std::ostream& out, const std::filesystem::path& path)
    {
        std::string path_str = path.string();
        uint64_t len = path_str.length();
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        out.write(path_str.c_str(), len);
    }

    static std::filesystem::path ReadPath(std::istream& in)
    {
        uint64_t len;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        std::string path_str(len, '\0');
        in.read(&path_str[0], len);
        return std::filesystem::path(path_str);
    }

    void AssetMetadata::Serialize(std::ostream& out) const
    {
        out.write(reinterpret_cast<const char*>(&Handle), sizeof(Handle));
        out.write(reinterpret_cast<const char*>(&Type), sizeof(Type));
        WritePath(out, SourceFilePath);
        WritePath(out, CookedFilePath);
        out.write(reinterpret_cast<const char*>(&SourceFileTimestamp), sizeof(SourceFileTimestamp));
        out.write(reinterpret_cast<const char*>(&MetaFileTimestamp), sizeof(MetaFileTimestamp));
    }

    void AssetMetadata::Deserialize(std::istream& in)
    {
        in.read(reinterpret_cast<char*>(&Handle), sizeof(Handle));
        in.read(reinterpret_cast<char*>(&Type), sizeof(Type));
        SourceFilePath = ReadPath(in);
        CookedFilePath = ReadPath(in);
        in.read(reinterpret_cast<char*>(&SourceFileTimestamp), sizeof(SourceFileTimestamp));
        in.read(reinterpret_cast<char*>(&MetaFileTimestamp), sizeof(MetaFileTimestamp));
    }

    void AssetMetadata::Serialize(ISerializer* serializer) const
    {
        Handle.Serialize(serializer);
        serializer->SerializeValue("Type", Type);
        serializer->SerializeValue("SourceFilePath", SourceFilePath);
        serializer->SerializeValue("CookedFilePath", CookedFilePath);
        serializer->SerializeValue("SourceFileTimestamp", SourceFileTimestamp);
        serializer->SerializeValue("MetaFileTimestamp", MetaFileTimestamp);
    }

    void AssetMetadata::Deserialize(IDeserializer* deserializer)
    {
        Handle.Deserialize(deserializer);
        
    }
}
