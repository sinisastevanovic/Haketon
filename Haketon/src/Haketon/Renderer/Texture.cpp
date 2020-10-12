#include "hkpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Haketon
{
    Ref<Texture2D> Texture2D::Create(const std::string& path, bool UseNearestFiltering)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:		HK_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::API::OpenGL:		return CreateRef<OpenGLTexture2D>(path, UseNearestFiltering);
        }

        HK_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:		HK_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::API::OpenGL:		return CreateRef<OpenGLTexture2D>(width, height);
        }

        HK_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}

