#pragma once
#include "OrthographicCamera.h"

namespace Haketon
{
    class Texture2D;

    class Renderer2D
    {
    public:
        static void Init();
        static void Shutdown();
        
        static void BeginScene(const OrthographicCamera& camera);
        static void EndScene();
        static void Flush();

        // TODO: Maybe instead of vec3 for Z-Order use a single float as optional parameter. This would erase some of these funtions...
        // Primitives
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size = {1.0f, 1.0f}, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f});
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size = {1.0f, 1.0f}, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f});
        static void DrawQuad(const glm::vec2& position, const Ref<Texture2D>& texture, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f}, float tilingFactor = 1.0f);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f}, float tilingFactor = 1.0f);

        // TODO: Make rotated quad work with batched rendering
        static void DrawRotatedQuad(const glm::vec2& position, const float rotation, const glm::vec2& size = {1.0f, 1.0f}, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f});
        static void DrawRotatedQuad(const glm::vec3& position, const float rotation, const glm::vec2& size = {1.0f, 1.0f}, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f});
        static void DrawRotatedQuad(const glm::vec2& position, const float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f}, float TilingFactor = 1.0f);
        static void DrawRotatedQuad(const glm::vec3& position, const float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f}, float TilingFactor = 1.0f);

    };
}

