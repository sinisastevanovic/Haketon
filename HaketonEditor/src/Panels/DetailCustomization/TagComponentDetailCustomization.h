#pragma once
#include "IDetailCustomization.h"

namespace Haketon
{
    class TagComponentDetailCustomization : public IDetailCustomization
    {
    public:
        void CustomizeDetails(rttr::instance Instance) override;
    };

}
