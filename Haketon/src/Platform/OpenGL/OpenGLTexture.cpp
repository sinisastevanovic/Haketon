#include "hkpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"
#include <fstream>

namespace Haketon
{
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
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
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
        in.read(reinterpret_cast<char*>(&width), sizeof(uint32_t));
        in.read(reinterpret_cast<char*>(&height), sizeof(uint32_t));
        in.read(reinterpret_cast<char*>(&format), sizeof(uint32_t));

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

        uint32_t rendererID;
        glCreateTextures(GL_TEXTURE_2D, 1, &rendererID);
        glTextureStorage2D(rendererID, 1, internalFormat, width, height);
        glTextureParameteri(rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // TODO: Save and load these settings somehow
        glTextureParameteri(rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureSubImage2D(rendererID, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, pixelData.data());

        HK_CORE_INFO("Loaded texture {}, ID: {}", cookedPath.string(), rendererID);
        return CreateRef<OpenGLTexture2D>(width, height, rendererID, internalFormat, dataFormat);
    }

    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, uint32_t rendererID, GLenum internalFormat, GLenum dataFormat)
        : m_Width(width), m_Height(height), m_RendererID(rendererID), m_InternalFormat(internalFormat), m_DataFormat(dataFormat)
    {
    }
}
