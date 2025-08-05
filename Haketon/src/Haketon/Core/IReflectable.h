#pragma once

#include <rttr/type>

namespace Haketon
{
    CLASS()
    class IReflectable
    {
    public:
        IReflectable() = default;
        virtual ~IReflectable() = default;

        RTTR_ENABLE()
    };
}