#include "hkpch.h"
#include "TextureImporter.h"

#include "stb_image.h"
#include <fstream>

#include "Haketon/Core/PathUtils.h"

namespace Haketon
{
    bool TextureImporter::Import(const std::filesystem::path& sourcePath, AssetMetadata& outMetadata)
    {
        HK_CORE_INFO("Importing texture from: {}", sourcePath.string());

        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);

        stbi_uc* data = stbi_load(sourcePath.string().c_str(), &width, &height, &channels, 0);    
        if (!data)
        {
            HK_CORE_ERROR("TextureImporter: Failed to load image file {}", sourcePath.string());
            return false;
        }

        std::filesystem::path cookedPath = PathUtils::GetGameCachePath() / "assets";
        cookedPath /= outMetadata.Handle.ToString();
        cookedPath.replace_extension(".htex");
        std::filesystem::create_directories(cookedPath.parent_path());

        std::ofstream out(cookedPath, std::ios::binary | std::ios::trunc);
        if (!out.is_open())
        {
            stbi_image_free(data);
            HK_CORE_ERROR("TextureImporter: Failed to open cooked file for writing: {}", cookedPath.string());
            return false;
        }

        // --- .htex Binary Format ---
        // TODO: In a real engine, this is where you would perform GPU texture compression (BCn/DXT).
        // For now, we'll write a simple uncompressed format.
        uint64_t handleInt = static_cast<uint64_t>(outMetadata.Handle);
        out.write(reinterpret_cast<const char*>(&handleInt), sizeof(uint64_t));
        out.write(reinterpret_cast<const char*>(&width), sizeof(uint32_t));
        out.write(reinterpret_cast<const char*>(&height), sizeof(uint32_t));
        out.write(reinterpret_cast<const char*>(&channels), sizeof(uint32_t));

        size_t dataSize = (size_t)width * height * channels;
        out.write(reinterpret_cast<const char*>(data), dataSize);

        stbi_image_free(data);
        out.close();

        outMetadata.CookedFilePath = cookedPath;
        return true;
    }
}

