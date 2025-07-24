#pragma once

#include <rttr/type>

namespace Haketon
{
    class IDetailCustomization
    {
    public:
        virtual ~IDetailCustomization() {}

        virtual void CustomizeDetails(rttr::instance Instance) = 0;

    };

    class IPropertyDetailCustomization
    {
    public:
        virtual ~IPropertyDetailCustomization() {}

        virtual bool CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly = false) = 0;
    };
}
