#pragma once

#include <rttr/type>

class IDetailCustomization
{
public:
    virtual ~IDetailCustomization() {}

    virtual void CustomizeDetails(rttr::instance Instance) = 0;

};