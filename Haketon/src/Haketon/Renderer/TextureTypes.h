#pragma once

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

enum class TextureWrap
{
    None = 0,
    Clamp,
    Repeat
};

enum class TextureFilter
{
    None = 0,
    Linear,
    Nearest,
    Cubic
};

struct TextureProperties
{
    TextureWrap SamplerWrap = TextureWrap::Repeat;
    TextureFilter SamplerFilter = TextureFilter::Linear;
    bool GenerateMips = true;
    bool Anisotropy = true;
    bool SRGB = false;
};