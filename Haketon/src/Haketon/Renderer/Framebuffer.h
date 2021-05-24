#pragma once

#include "Haketon/Core/Core.h"

#include <glm/glm.hpp>

namespace Haketon
{
    enum class FramebufferTextureFormat
    {
        None = 0,

        // Color
        RGBA8,
        RED_INTEGER,
        
        // Depth/stencil
        DEPTH24STENCIL8,

        // Defaults
        Depth = DEPTH24STENCIL8
    };

    struct FramebufferTextureSpecification
    {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat Format)
            : TextureFormat(Format) {}

        FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
        //TODO: filtering/wrap
    };

    struct FramebufferAttachmentSpecification
    {
        FramebufferAttachmentSpecification() = default;
        FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> AttachmentSpecs)
            : Attachments(AttachmentSpecs) {}
        
        std::vector<FramebufferTextureSpecification> Attachments;
    };
    
    struct FramebufferSpecification
    {
        uint32_t Width = 0, Height = 0;
        FramebufferAttachmentSpecification Attachments;
        uint32_t Samples = 1;

        bool SwapChainTarget = false;
    };
    
    class Framebuffer
    {
    public:
        virtual ~Framebuffer() = default;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual int ReadPixel(uint32_t AttachmentIndex, int X, int Y) = 0;

        virtual void ClearAttachment(uint32_t AttachmentIndex, int Value) = 0;

        virtual uint32_t GetColorAttachmentRendererID(uint32_t Index = 0) const = 0;
        
        virtual const FramebufferSpecification& GetSpecification() const = 0;
        
        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
    };
}

