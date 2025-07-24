#pragma once
#include "IDetailCustomization.h"
#include "Haketon/Math/Math.h"

namespace Haketon
{
    class QuatPropertyDetailCustomization : public IPropertyDetailCustomization
    {
    public:
        bool CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly = false) override;

    private:
        bool DrawQuatControl(const std::string& label, FQuat& values, float resetValue = 0.0f, float columnWidth = 100.0f);
        bool DrawEulerControl(const std::string& label, FVec3& eulerAngles);
    };
}
