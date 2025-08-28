#pragma once
#include "Haketon/Asset/AssetSettings.h"

namespace Haketon
{
    enum class ImageFormat
    {
        None = 0,
        RED8UN,
        RED8UI,
        RED16UI,
        RED32UI,
        RED32F,
        RG8,
        RG16F,
        RG32F,
        RGB,
        RGBA,
        RGBA16F,
        RGBA32F,

        B10R11G11UF,

        SRGB,

        DEPTH32FSTENCIL8UINT,
        DEPTH32F,
        DEPTH24STENCIL8,
    };

    ENUM()
    enum class TextureWrap
    {
        None = 0,
        Clamp,
        Repeat
    };

    ENUM()
    enum class TextureFilter
    {
        None = 0,
        Linear,
        Nearest,
        Cubic
    };

    CLASS()
    class TextureProperties : public AssetSettings
    {
    public:
        PROPERTY()
        TextureWrap SamplerWrap = TextureWrap::Repeat;
        PROPERTY()
        TextureFilter SamplerFilter = TextureFilter::Linear;
        PROPERTY()
        bool GenerateMips = true;
        PROPERTY()
        bool Anisotropy = true;
        PROPERTY()
        bool SRGB = false;

        RTTR_ENABLE(AssetSettings)
    };
}
