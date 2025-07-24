#pragma once
#include "IDetailCustomization.h"
#include "Haketon/Math/Math.h"

namespace Haketon
{
    class Vec3PropertyDetailCustomization : public IPropertyDetailCustomization
    {
    public:
        bool CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly = false) override;

    private:
        bool DrawVec3Control(const std::string& label, FVec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
    };
}
