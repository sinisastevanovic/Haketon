#pragma once
#include "DetailCustomization/IDetailCustomization.h"
#include "Haketon/Math/Math.h"

namespace Haketon
{
    class Vec2PropertyDetailCustomization : public IPropertyDetailCustomization
    {
    public:
        bool CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly = false) override;

    private:
        bool DrawVec2Control(const std::string& label, Haketon::FVec2& values, float resetValue = 0.0f, float columnWidth = 100.0f);
    };
}

