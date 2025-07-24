#pragma once

#include <glm/glm.hpp>

namespace Haketon
{
    typedef glm::vec2 FVec2;
    typedef glm::vec3 FVec3;
    typedef glm::vec4 FVec4;
    typedef glm::ivec2 FInt2;
    typedef glm::ivec3 FInt3;
    typedef glm::ivec4 FInt4;
    typedef glm::quat FQuat;
    
    // Color wrapper around glm::vec4 for RGBA color values
    struct FColor
    {
        glm::vec4 rgba;
            
        FColor() : rgba(1.0f, 1.0f, 1.0f, 1.0f) {}
        FColor(float r, float g, float b, float a = 1.0f) : rgba(r, g, b, a) {}
        FColor(const glm::vec4& vec) : rgba(vec) {}
        FColor(const FVec3& vec, float a = 1.0f) : rgba(vec, a) {}
            
        // Implicit conversion to glm::vec4
        operator glm::vec4() const { return rgba; }
        operator glm::vec4&() { return rgba; }
            
        // Component accessors
        float& r() { return rgba.r; }
        float& g() { return rgba.g; }
        float& b() { return rgba.b; }
        float& a() { return rgba.a; }
            
        const float& r() const { return rgba.r; }
        const float& g() const { return rgba.g; }
        const float& b() const { return rgba.b; }
        const float& a() const { return rgba.a; }

        float getR() const { return rgba.r; }
        float getG() const { return rgba.g; }
        float getB() const { return rgba.b; }
        float getA() const { return rgba.a; }

        void setR(float r) { rgba.r = r; }
        void setG(float g) { rgba.g = g; }
        void setB(float b) { rgba.b = b; }
        void setA(float a) { rgba.a = a; }
            
        // Common color constants
        static FColor White() { return FColor(1.0f, 1.0f, 1.0f, 1.0f); }
        static FColor Black() { return FColor(0.0f, 0.0f, 0.0f, 1.0f); }
        static FColor Red() { return FColor(1.0f, 0.0f, 0.0f, 1.0f); }
        static FColor Green() { return FColor(0.0f, 1.0f, 0.0f, 1.0f); }
        static FColor Blue() { return FColor(0.0f, 0.0f, 1.0f, 1.0f); }
        static FColor Transparent() { return FColor(0.0f, 0.0f, 0.0f, 0.0f); }
    };

    namespace Math
    {
        bool DecomposeTransform(const glm::mat4& Transform, FVec3& OutTranslation, FVec3& OutRotation, FVec3& OutScale);
    }
}
