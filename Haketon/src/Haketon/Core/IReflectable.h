#pragma once

#include "Core.h"
#include <rttr/type>

namespace Haketon
{
    STRUCT()
    class IReflectable
    {
    public:
        IReflectable() = default;
        virtual ~IReflectable() = default;

        RTTR_ENABLE()
    };
}