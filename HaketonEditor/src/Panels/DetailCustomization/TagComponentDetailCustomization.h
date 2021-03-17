#pragma once
#include "IDetailCustomization.h"

class TagComponentDetailCustomization : public IDetailCustomization
{
public:
    void CustomizeDetails(rttr::instance Instance) override;
};
