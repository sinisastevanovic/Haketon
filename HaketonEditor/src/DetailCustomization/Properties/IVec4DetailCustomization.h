#pragma once
#include "DetailCustomization/IDetailCustomization.h"
#include "Haketon/Math/Math.h"

namespace Haketon
{
    class IVec4PropertyDetailCustomization : public IPropertyDetailCustomization
    {
    public:
        bool CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly = false) override;

    private:
        bool DrawIVec4Control(const std::string& label, FInt4& values, int resetValue = 0, float columnWidth = 100.0f);
    };
}
