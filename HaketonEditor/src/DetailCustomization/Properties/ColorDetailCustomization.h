#pragma once
#include "DetailCustomization/IDetailCustomization.h"
#include "Haketon/Math/Math.h"

namespace Haketon
{
    class ColorPropertyDetailCustomization : public IPropertyDetailCustomization
    {
    public:
        bool CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly = false) override;

    private:
        bool DrawColorControl(const std::string& label, Haketon::FColor& color);
    };
}