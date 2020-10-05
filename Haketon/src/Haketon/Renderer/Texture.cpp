#include "hkpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Haketon
{
    Ref<Texture2D> Texture2D::Create(const std::string& path)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:		HK_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::API::OpenGL:		return CreateRef<OpenGLTexture2D>(path);
        }

        HK_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}

