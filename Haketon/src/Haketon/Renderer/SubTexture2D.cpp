#include "hkpch.h"
#include "SubTexture2D.h"

namespace Haketon
{
    SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, const FVec2& min, const FVec2& max)
        : m_Texture(texture)
    {
        m_TexCoords[0] = { min.x, min.y };
        m_TexCoords[1] = { max.x, min.y };
        m_TexCoords[2] = { max.x, max.y };
        m_TexCoords[3] = { min.x, max.y };
    }

    Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture, const FVec2& coords, const FVec2& cellSize, const FVec2& spriteSize)
    {
        FVec2 min = { (coords.x * cellSize.x) / texture->GetWidth(),(coords.y * cellSize.y) / texture->GetHeight() };
        FVec2 max = { ((coords.x + spriteSize.x) * cellSize.x) / texture->GetWidth(),((coords.y + spriteSize.y) * cellSize.y) / texture->GetHeight() };
        return CreateRef<SubTexture2D>(texture, min, max);
    }
}
