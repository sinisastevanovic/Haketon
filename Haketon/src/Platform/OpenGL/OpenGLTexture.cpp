#include "hkpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"
#include <fstream>

namespace Haketon
{
    namespace OpenGLTextureHelpers
    {
        GLenum GetOpenGLFiler(TextureFilter filter)
        {
            switch (filter)
            {
                case TextureFilter::Linear: return GL_LINEAR;
                case TextureFilter::Nearest: return GL_NEAREST;
            }
            HK_CORE_ASSERT(false, "Unknown filter type");
            return 0;
        }

        GLenum GetOpenGLWrapMode(TextureWrap wrap)
        {
            switch (wrap)
            {
                case TextureWrap::Repeat: return GL_REPEAT;
                case TextureWrap::Clamp: return GL_CLAMP_TO_EDGE;
            }
            HK_CORE_ASSERT(false, "Unknown wrap type");
            return 0;
        }
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool UseNearestFiltering)
    {
        HK_PROFILE_FUNCTION(); // TODO: Go through all renderer classes (also under platform) and replace profiler with renderer specific profiler

        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);

        stbi_uc* data = nullptr;
        {
            HK_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string&, bool)");
            data = stbi_load(path.c_str(), &width, &height, &channels, 0);    
        }

        HK_CORE_ASSERT(data, "Failed to load image!");
        m_Width = width;
        m_Height = height;

        GLenum internalFormat = 0, dataFormat = 0;
        if(channels == 4)
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }
        else if(channels == 3)
        {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }

        m_InternalFormat = internalFormat;
        m_DataFormat = dataFormat;

        HK_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");
        
        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, UseNearestFiltering ? GL_NEAREST : GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, UseNearestFiltering ? GL_NEAREST : GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
        : m_Width(width), m_Height(height)
    {
        HK_PROFILE_FUNCTION();

        m_InternalFormat = GL_RGBA8;
        m_DataFormat = GL_RGBA;
        
        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        HK_PROFILE_FUNCTION();

        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLTexture2D::SetData(void* data, uint32_t size)
    {
        HK_PROFILE_FUNCTION();

        uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3; 
        HK_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
        glPixelStorei(GL_UNPACK_ALIGNMENT, (bpp == 3) ? 1 : 4);
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const
    {
        HK_PROFILE_FUNCTION();

        glBindTextureUnit(slot, m_RendererID);
    }

    Ref<Texture2D> OpenGLTexture2D::Create(const std::filesystem::path& cookedPath)
    {
        std::ifstream in(cookedPath, std::ios::binary);
        if (!in.is_open())
        {
            HK_CORE_ERROR("Failed to open cooked texture file: {}", cookedPath.string());
            return nullptr;
        }

        uint32_t width, height, format;
        TextureProperties properties;
        in.read(reinterpret_cast<char*>(&width), sizeof(uint32_t));
        in.read(reinterpret_cast<char*>(&height), sizeof(uint32_t));
        in.read(reinterpret_cast<char*>(&format), sizeof(uint32_t));
        in.read(reinterpret_cast<char*>(&properties.SamplerWrap), sizeof(TextureWrap));
        in.read(reinterpret_cast<char*>(&properties.SamplerFilter), sizeof(TextureFilter));
        in.read(reinterpret_cast<char*>(&properties.GenerateMips), sizeof(bool));
        in.read(reinterpret_cast<char*>(&properties.Anisotropy), sizeof(bool));
        in.read(reinterpret_cast<char*>(&properties.SRGB), sizeof(bool));

        size_t dataSize = (size_t)width * height * format;
        std::vector<unsigned char> pixelData(dataSize);
        in.read(reinterpret_cast<char*>(pixelData.data()), dataSize);
        in.close();

        GLenum internalFormat = 0, dataFormat = 0;
        if(format == 4)
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }
        else if(format == 3)
        {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }
        else
        {
            HK_CORE_ERROR("Unsupported texture format for: {}", cookedPath.string());
        }

        uint32_t rendererID;
        glCreateTextures(GL_TEXTURE_2D, 1, &rendererID);
        if (properties.GenerateMips)
        {
            GLsizei mipLevels = static_cast<GLsizei>(std::floor(std::log2(std::max(width, height)))) + 1;
            glTextureStorage2D(rendererID, mipLevels, internalFormat, width, height);
        }
        else
        {
            glTextureStorage2D(rendererID, 1, internalFormat, width, height);
        }

        glTextureParameteri(rendererID, GL_TEXTURE_MIN_FILTER, OpenGLTextureHelpers::GetOpenGLFiler(properties.SamplerFilter));
        glTextureParameteri(rendererID, GL_TEXTURE_MAG_FILTER, OpenGLTextureHelpers::GetOpenGLFiler(properties.SamplerFilter));
        glTextureParameteri(rendererID, GL_TEXTURE_WRAP_S, OpenGLTextureHelpers::GetOpenGLWrapMode(properties.SamplerWrap));
        glTextureParameteri(rendererID, GL_TEXTURE_WRAP_T, OpenGLTextureHelpers::GetOpenGLWrapMode(properties.SamplerWrap));
        glTextureSubImage2D(rendererID, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, pixelData.data());

        if (properties.GenerateMips)
        {
            glGenerateTextureMipmap(rendererID);
        }

        // TODO: We need to enable the extension
        /*if (properties.Anisotropy)
        {
            GLfloat maxAnisotropy;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            glTextureParameterf(rendererID, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        }*/

        HK_CORE_INFO("Loaded texture {}, ID: {}", cookedPath.string(), rendererID);
        return CreateRef<OpenGLTexture2D>(width, height, rendererID, internalFormat, dataFormat, properties);
    }

    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, uint32_t rendererID, GLenum internalFormat, GLenum dataFormat, TextureProperties properties)
        : m_Width(width), m_Height(height), m_RendererID(rendererID), m_InternalFormat(internalFormat), m_DataFormat(dataFormat)
    {
        m_Properties = properties;
    }
}
