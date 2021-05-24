#include "hkpch.h"
#include "OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace Haketon
{
    static const uint32_t s_MaxFramebufferSize = 8192;

    namespace Utils
    {
        static GLenum TextureTarget(bool Multisampled)
        {
            return Multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        }
        
        static void CreateTextures(bool Multisampled, uint32_t* OutID, uint32_t Count)
        {
            glCreateTextures(TextureTarget(Multisampled), Count, OutID);
        }

        static void BindTexture(bool Multisampled, uint32_t ID)
        {
            glBindTexture(TextureTarget(Multisampled), ID);
        }

        static void AttachColorTexture(uint32_t ID, int Samples, GLenum Format, uint32_t Width, uint32_t Height, int Index)
        {
            bool bMultisampled = Samples > 1;
            if(bMultisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, Format, Width, Height, GL_FALSE);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Index, TextureTarget(bMultisampled), ID, 0);
        }

        static void AttachDepthTexture(uint32_t ID, int Samples, GLenum Format, GLenum AttachmentType, uint32_t Width, uint32_t Height)
        {
            bool bMultisampled = Samples > 1;
            if(bMultisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, Format, Width, Height, GL_FALSE);
            }
            else
            {
                glTexStorage2D(GL_TEXTURE_2D, 1, Format, Width, Height);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, AttachmentType, TextureTarget(bMultisampled), ID, 0);
        }
        
        static bool IsDepthFormat(FramebufferTextureFormat Format)
        {
            switch(Format)
            {
            case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
            }

            return false;
        }

    }
    
    
    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
        : m_Specification(spec)
    {
        for(auto Spec : m_Specification.Attachments.Attachments)
        {
            if(!Utils::IsDepthFormat(Spec.TextureFormat))
                m_ColorAttachmentSpecs.emplace_back(Spec);
            else
                m_DepthAttachmentSpec = Spec;         
        }
        
        Invalidate();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        glDeleteFramebuffers(1, &m_RendererID);
        glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
        glDeleteTextures(1, &m_DepthAttachment);
    }

    void OpenGLFramebuffer::Invalidate()
    {
        if(m_RendererID)
        {
            glDeleteFramebuffers(1, &m_RendererID);
            glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
            glDeleteTextures(1, &m_DepthAttachment);

            m_ColorAttachments.clear();
            m_DepthAttachment = 0;
        }
        
        glCreateFramebuffers(1, &m_RendererID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        bool bMultiSample = m_Specification.Samples > 1;

        // Attachments
        if(m_ColorAttachmentSpecs.size())
        {
            m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
            Utils::CreateTextures(bMultiSample, m_ColorAttachments.data(), m_ColorAttachments.size());
            
            for(size_t i = 0; i < m_ColorAttachments.size(); i++)
            {
                Utils::BindTexture(bMultiSample, m_ColorAttachments[i]);
                switch(m_ColorAttachmentSpecs[i].TextureFormat)
                {
                case FramebufferTextureFormat::RGBA8:
                    Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, m_Specification.Width, m_Specification.Height, i);
                    break;
                }
            }
        }

        if(m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
        {
            Utils::CreateTextures(bMultiSample, &m_DepthAttachment, 1);
            Utils::BindTexture(bMultiSample, m_DepthAttachment);
            switch(m_DepthAttachmentSpec.TextureFormat)
            {
            case FramebufferTextureFormat::DEPTH24STENCIL8:
                Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                break;
            }
        }
        
        if(m_ColorAttachments.size() > 1)
        {
            HK_CORE_ASSERT(m_ColorAttachments.size() <= 4);

            GLenum Buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
            glDrawBuffers(m_ColorAttachments.size(), Buffers);           
        }
        else if(m_ColorAttachments.empty())
        {
            // Only depth-pass
            glDrawBuffer(GL_NONE);
        }

        HK_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glViewport(0, 0, m_Specification.Width, m_Specification.Height);
    }

    void OpenGLFramebuffer::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
    {
        if(width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
        {
            HK_CORE_WARN("Attempting to resize Framebuffer to ({0} | {1})", width, height);
            return;
        }
        
        m_Specification.Width = width;
        m_Specification.Height = height;

        Invalidate();
    }
}   