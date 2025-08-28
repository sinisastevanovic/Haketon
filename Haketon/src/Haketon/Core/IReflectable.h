#pragma once

#include "Core.h"
#include <rttr/type>

namespace Haketon
{
    CLASS()
    class HK_API IReflectable
    {
    public:
        IReflectable() = default;
        virtual ~IReflectable() = default;

        RTTR_ENABLE()
    };
}