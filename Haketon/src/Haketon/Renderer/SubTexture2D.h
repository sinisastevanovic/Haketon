#pragma once

#include <glm/glm.hpp>

#include "Texture.h"

namespace Haketon {
   class SubTexture2D
    {
    public:
       SubTexture2D(const Ref<Texture2D>& texture, const FVec2& min, const FVec2& max);

       const Ref<Texture2D> GetTexture() const { return m_Texture; }
       const FVec2* GetTexCoords() const { return m_TexCoords; }

       static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& texture, const FVec2& coords, const FVec2& cellSize, const FVec2& spriteSize = { 1.0f, 1.0f });
       
   private:
       Ref<Texture2D> m_Texture;

       FVec2 m_TexCoords[4];
    
    };

}
