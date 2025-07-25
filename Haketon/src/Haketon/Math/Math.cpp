﻿#include "hkpch.h"
#include "Math.h"

//#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include "glm/gtx/quaternion.hpp"

namespace Haketon
{
    namespace Math
    {
        bool DecomposeTransform(const glm::mat4& Transform, FVec3& OutTranslation, FVec3& OutRotation, FVec3& OutScale)
        {
            // From glm::decompose in matrix_decompose.inl

            using namespace glm;
            using T = float;

            mat4 LocalMatrix(Transform);

            // Normalize the matrix
            if(epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
                return false;

            // First, isolate perspective. This is the messiest...
            if(epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
               epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
               epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
            {
                // Clear the perspective partition
                LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
                LocalMatrix[3][3] = static_cast<T>(1);
            }

            // Next take care of translation
            OutTranslation = vec3(LocalMatrix[3]);
            LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

            vec3 Row[3], Pdum3;

            // Now get scale and shear
            for(length_t i = 0; i < 3; ++i)
                for(length_t j = 0; j < 3; ++j)
                    Row[i][j] = LocalMatrix[i][j];

            // Compute X scale factor and normalize first row.
            OutScale.x = length(Row[0]);
            Row[0] = detail::scale(Row[0], static_cast<T>(1));
            OutScale.y = length(Row[1]);
            Row[1] = detail::scale(Row[1], static_cast<T>(1));
            OutScale.z = length(Row[2]);
            Row[2] = detail::scale(Row[2], static_cast<T>(1));

            // At this point, the matrix (in rows[]) is orthonormal.
            // Check for a coordinate system flip. If the determinant
            // is -1, then negate the matrix and the scaling factors.

#if 0
            Pdum3 = cross(Row[1], Row[2]);
            if(dot(Row[0], Pdum3) < 0)
            {
                for(length_t i = 0; i < 3; i++)
                {
                    OutScale[i] *= static_cast<T>(-1);
                    Row[i] *= static_cast<T>(-1);
                }
            }
#endif

            OutRotation.y = asin(-Row[0][2]);
            if(cos(OutRotation.y) != 0)
            {
                OutRotation.x = atan2(Row[1][2], Row[2][2]);
                OutRotation.z = atan2(Row[0][1], Row[0][0]);
            }
            else
            {
                OutRotation.x = atan2(-Row[2][0], Row[1][1]);
                OutRotation.z = 0;
            }

            return true;
        }
    }
}
