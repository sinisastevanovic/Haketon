#pragma once

#include <glad/glad.h>
#include "Haketon/Renderer/Texture.h"

namespace Haketon
{
    class OpenGLTexture2D : public Texture2D
    {
    public:
        OpenGLTexture2D(const std::string& path, bool UseNearestFiltering = false); // TODO: replace bool with enum?
        OpenGLTexture2D(uint32_t width, uint32_t height);
        virtual ~OpenGLTexture2D();

        virtual void SetData(void* data, uint32_t size) override;

        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }

        virtual void Bind(uint32_t slot = 0) const override;

    private:
        std::string m_Path;
        uint32_t m_Width = 1;
        uint32_t m_Height = 1;
        uint32_t m_RendererID;
        GLenum m_InternalFormat, m_DataFormat;
    };
}

