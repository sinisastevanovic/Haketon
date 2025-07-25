﻿#include "hkpch.h"
#include "Renderer2D.h"

#include "RenderCommand.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include "Texture.h"
#include "VertexArray.h"
#include "Haketon/Scene/Components.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Haketon
{
    struct QuadVertex
    {
        FVec3 Position;
        FColor Color;
        FVec2 TexCoord;
        float TexIndex;
        float TilingFactor;

        // Editor-only
        int EntityID;
    };
    
    struct Renderer2DData
    {
        static const uint32_t MaxQuads = 1000000;
        static const uint32_t MaxVertices = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        static const uint32_t MaxTextureSlots = 32; // TODO: RenderCaps
        
        Ref<VertexArray> QuadVertexArray;
        Ref<VertexBuffer> QuadVertexBuffer;
        Ref<Shader> TextureShader;
        Ref<Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        FVec4 QuadVertexPositions[4];

        Renderer2D::Statistics Stats;

        struct CameraData
        {
            glm::mat4 ViewProjection;
        };

        CameraData CameraBuffer;
        Ref<UniformBuffer> CameraUniformBuffer;
    };

    static Renderer2DData s_Data;
    
    void Renderer2D::Init()
    {
        HK_PROFILE_FUNCTION();

        s_Data.QuadVertexArray = VertexArray::Create();

        s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
        s_Data.QuadVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float, "a_TexIndex" },
            { ShaderDataType::Float, "a_TilingFactor" },
            { ShaderDataType::Int, "a_EntityID" }
        });
        s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

        s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];
        
        uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

        uint32_t offset = 0;
        for(uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }
        
        Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
        s_Data.QuadVertexArray->SetIndexBuffer(quadIB);
        delete[] quadIndices;

        s_Data.WhiteTexture = Texture2D::Create(1, 1);
        uint32_t texData = 0xffffffff;
        s_Data.WhiteTexture->SetData(&texData, sizeof(uint32_t));

        /*int32_t samplers[s_Data.MaxTextureSlots];
        for(uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            samplers[i] = i;*/

        s_Data.TextureShader = Shader::Create("assets/shaders/Texture.glsl");

        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

        s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
    }

    void Renderer2D::Shutdown()
    {
        HK_PROFILE_FUNCTION();

        delete[] s_Data.QuadVertexBufferBase;
    }
   
    /*void Renderer2D::BeginScene(const OrthographicCamera& camera)
    {
        HK_PROFILE_FUNCTION();

        s_Data.TextureShader->Bind();
        s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

        StartBatch();
    }*/
    
    void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        HK_PROFILE_FUNCTION();

        glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
        
        s_Data.TextureShader->Bind();
        s_Data.TextureShader->SetMat4("u_ViewProjection", viewProj);

        StartBatch();
    }


    void Renderer2D::BeginScene(const EditorCamera& Camera)
    {
        HK_PROFILE_FUNCTION();

        s_Data.CameraBuffer.ViewProjection = Camera.GetViewProjection();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

        StartBatch();
    }

    void Renderer2D::EndScene()
    {
        HK_PROFILE_FUNCTION();

        Flush();
    }

    void Renderer2D::StartBatch()
    {
        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::Flush()
    {
        if(s_Data.QuadIndexCount == 0)
            return; // Nothing to draw

        uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
        s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
        
        // Bind textures
        for(uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
            s_Data.TextureSlots[i]->Bind(i);       

        s_Data.TextureShader->Bind();
        
        RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
        s_Data.Stats.DrawCalls++;
    }

    void Renderer2D::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer2D::DrawQuad(const FVec3& position, const FVec2& size, const FColor& color)
    {
        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                                    glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        
        DrawQuad(transform, color);
    }

    void Renderer2D::DrawQuad(const Ref<Texture2D>& texture, const FVec3& position, const FVec2& size,
        const FColor& tintColor, float tilingFactor)
    {
        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                                    glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        
        DrawQuad(texture, transform, tintColor, tilingFactor);
    }

    void Renderer2D::DrawQuad(const Ref<SubTexture2D>& subTexture, const FVec3& position, const FVec2& size,
        const FColor& tintColor, float tilingFactor)
    {
        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                                    glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        
        DrawQuad(subTexture, transform, tintColor, tilingFactor);      
    }

    void Renderer2D::DrawRotatedQuad(const FVec3& position, const float rotation, const FVec2& size,
                                     const FColor& color)
    {
        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                                    glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                                    glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        
       DrawQuad(transform, color);
    }

    void Renderer2D::DrawRotatedQuad(const Ref<Texture2D>& texture, const FVec3& position, const float rotation, const FVec2& size,
        const FColor& tintColor, float tilingFactor)
    {
        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                                    glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                                    glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        
        DrawQuad(texture, transform, tintColor, tilingFactor);
    }

    void Renderer2D::DrawRotatedQuad(const Ref<SubTexture2D>& subTexture, const FVec3& position, const float rotation, const FVec2& size,
        const FColor& tintColor, float tilingFactor)
    {
        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                                    glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                                    glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        
        DrawQuad(subTexture, transform, tintColor, tilingFactor);
    }

    
    void Renderer2D::DrawQuad(const glm::mat4& transform, const FColor& color, int entityID)
    {
        HK_PROFILE_FUNCTION();

        constexpr size_t quadVertexCount = 4;
        const float textureIndex = 0.0f; // White Texture
        constexpr FVec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float tilingFactor = 1.0f;

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
            NextBatch();

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const Ref<Texture2D>& texture, const glm::mat4& transform, const FColor& tintColor,
        float tilingFactor, int entityID)
    {
        HK_PROFILE_FUNCTION();

        constexpr size_t quadVertexCount = 4;
        constexpr FVec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
            NextBatch();

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *texture.get()) // TODO: Check if we need .get()
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f)
        {
            if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                NextBatch();

            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = tintColor;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const Ref<SubTexture2D>& subTexture, const glm::mat4& transform,
        const FColor& tintColor, float tilingFactor, int entityID)
    {
        HK_PROFILE_FUNCTION();

        constexpr size_t quadVertexCount = 4;
        const FVec2* textureCoords = subTexture->GetTexCoords();
        const Ref<Texture2D> texture = subTexture->GetTexture();

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
            NextBatch();

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *texture.get()) // TODO: Un-Ugly this
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f)
        {
            if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                NextBatch();

            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = tintColor;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& Src, int EntityID)
    {
        DrawQuad(transform, Src.Color, EntityID);
    }

    Renderer2D::Statistics Renderer2D::GetStats()
    {
        return s_Data.Stats;
    }

    void Renderer2D::ResetStats()
    {
        memset(&s_Data.Stats, 0, sizeof(Statistics));
    }
}
