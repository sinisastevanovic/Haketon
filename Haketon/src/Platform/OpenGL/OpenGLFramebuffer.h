#pragma once
#include "Haketon/Renderer/Framebuffer.h"

namespace Haketon
{
    class OpenGLFramebuffer : public Framebuffer
    {
    public:
        OpenGLFramebuffer(const FramebufferSpecification& spec);
        virtual ~OpenGLFramebuffer();

        void Invalidate();

        virtual void Bind() override;
        virtual void Unbind() override;

        virtual void Resize(uint32_t width, uint32_t height) override;
        virtual int ReadPixel(uint32_t AttachmentIndex, int X, int Y) override;

        virtual uint32_t GetColorAttachmentRendererID(uint32_t Index = 0) const override { HK_CORE_ASSERT(Index < m_ColorAttachments.size()); return m_ColorAttachments[Index]; }
        virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; };
        
    private:
        uint32_t m_RendererID = 0;
        FramebufferSpecification m_Specification;

        std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
        FramebufferTextureSpecification m_DepthAttachmentSpec;

        std::vector<uint32_t> m_ColorAttachments;
        uint32_t m_DepthAttachment = 0; 
    };
}

