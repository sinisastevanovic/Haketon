#pragma once

#include <glm/glm.hpp>

namespace Haketon
{
    namespace Math
    {
        bool DecomposeTransform(const glm::mat4& Transform, glm::vec3& OutTranslation, glm::vec3& OutRotation, glm::vec3& OutScale);
    }
}
