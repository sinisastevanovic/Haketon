#pragma once
#include "IDetailCustomization.h"
#include <glm/glm.hpp>

namespace Haketon
{
    class IVec2PropertyDetailCustomization : public IPropertyDetailCustomization
    {
    public:
        bool CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly = false) override;

    private:
        bool DrawIVec2Control(const std::string& label, glm::ivec2& values, int resetValue = 0, float columnWidth = 100.0f);
    };
}