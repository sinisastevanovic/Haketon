#pragma once

#include "Haketon/Core/Core.h"
#include "Haketon/Core/IReflectable.h"
#include <rttr/registration.h>

namespace Haketon
{
    CLASS()
    class HK_API AssetSettings : public IReflectable
    {
    public:
        AssetSettings() = default;
        virtual ~AssetSettings() = default;

        RTTR_ENABLE(IReflectable)
    };
}