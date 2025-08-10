#pragma once

#include <vector>
#include "Layer.h"

namespace Haketon
{
    class IApplicationContext
    {
    public:
        virtual ~IApplicationContext() = default;

        std::vector<Layer*> CreatedLayers;
        std::vector<Layer*> CreatedOverlays;
    };
}