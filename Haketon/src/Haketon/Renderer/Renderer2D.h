#pragma once
#include "Camera.h"
#include "OrthographicCamera.h"
#include "SubTexture2D.h"

#include "EditorCamera.h"

namespace Haketon
{
    class Texture2D;
    class SpriteRendererComponent;
    
    class Renderer2D
    {
    public:

        static void Init();
        static void Shutdown();
        
        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void BeginScene(const EditorCamera& Camera);
        static void BeginScene(const OrthographicCamera& camera); // TODO: remove
        static void EndScene();
        static void Flush();

        
        
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size = {1.0f, 1.0f}, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f});
        static void DrawQuad(const Ref<Texture2D>& texture, const glm::vec3& position, const glm::vec2& size = {1.0f, 1.0f}, const glm::vec4& tintColor = {1.0f, 1.0f, 1.0f, 1.0f}, float tilingFactor = 1.0f);
        static void DrawQuad(const Ref<SubTexture2D>& subTexture, const glm::vec3& position, const glm::vec2& size = {1.0f, 1.0f}, const glm::vec4& tintColor = {1.0f, 1.0f, 1.0f, 1.0f}, float tilingFactor = 1.0f);

        // Rotation is in radians
        static void DrawRotatedQuad(const glm::vec3& position, const float rotation, const glm::vec2& size = {1.0f, 1.0f}, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f});
        static void DrawRotatedQuad(const Ref<Texture2D>& texture, const glm::vec3& position, const float rotation, const glm::vec2& size = {1.0f, 1.0f}, const glm::vec4& tintColor = {1.0f, 1.0f, 1.0f, 1.0f}, float tilingFactor = 1.0f);
        static void DrawRotatedQuad(const Ref<SubTexture2D>& subTexture, const glm::vec3& position, const float rotation, const glm::vec2& size = {1.0f, 1.0f}, const glm::vec4& tintColor = {1.0f, 1.0f, 1.0f, 1.0f}, float tilingFactor = 1.0f);

        static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
        static void DrawQuad(const Ref<Texture2D>& texture, const glm::mat4& transform, const glm::vec4& tintColor, float tilingFactor = 1.0f, int entityID = -1);
        static void DrawQuad(const Ref<SubTexture2D>& subTexture, const glm::mat4& transform, const glm::vec4& tintColor, float tilingFactor, int entityID = -1);

        static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& Src, int EntityID);
        
        // Stats
        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t QuadCount = 0;

            uint32_t GetTotalVertexCount() { return QuadCount * 4; }
            uint32_t GetTotalIndexCount() { return QuadCount * 6; }
        };

        static Statistics GetStats();
        static void ResetStats();

    private:
        static void StartBatch();
        static void NextBatch();
        
    };
}

