#pragma once

#include <string>

#include "Haketon/Core/Core.h"
#include "Haketon/Asset/Asset.h"
#include <filesystem>

namespace Haketon
{
    class HK_API Texture : public Asset
    {
    public:
        ~Texture() override = default;

        AssetType GetType() const override { return AssetType::Texture; }
        
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetRendererID() const = 0;

        virtual void SetData(void* data, uint32_t size) = 0;
        
        virtual void Bind(uint32_t slot = 0) const = 0;

        virtual bool operator==(const Texture& other) const = 0;
    };

    class HK_API Texture2D : public Texture
    {
    public:
        static Ref<Texture2D> Create(const std::filesystem::path& cookedPath);
        // TODO: What to do with these? We still want to be able to create a texture directly from a file or an empty texture...
        static Ref<Texture2D> Create(const std::string& path = "../Haketon/assets/textures/T_UVChecker_512px.png", bool UseNearestFiltering = false); // Replace bool with enum
        static Ref<Texture2D> Create(uint32_t width, uint32_t height);
    };
}
