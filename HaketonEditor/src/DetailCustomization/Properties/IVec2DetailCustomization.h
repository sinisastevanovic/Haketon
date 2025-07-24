#pragma once
#include "DetailCustomization/IDetailCustomization.h"
#include "Haketon/Math/Math.h"

namespace Haketon
{
    class IVec2PropertyDetailCustomization : public IPropertyDetailCustomization
    {
    public:
        bool CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly = false) override;

    private:
        bool DrawIVec2Control(const std::string& label, FInt2& values, int resetValue = 0, float columnWidth = 100.0f);
    };
}