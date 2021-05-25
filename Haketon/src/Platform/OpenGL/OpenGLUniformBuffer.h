#pragma once

#include "Haketon/Renderer/UniformBuffer.h"

namespace Haketon
{
    class OpenGLUniformBuffer : public UniformBuffer
    {
    public:
        OpenGLUniformBuffer(uint32_t size, uint32_t binding);
        virtual ~OpenGLUniformBuffer();

        virtual void SetData(const void* data, uint32_t size, uint32_t offset) override;

    private:
        uint32_t m_RendererID = 0;
    
    };    
}