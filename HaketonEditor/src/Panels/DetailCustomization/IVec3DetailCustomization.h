#pragma once
#include "IDetailCustomization.h"
#include <glm/glm.hpp>

class IVec3PropertyDetailCustomization : public IPropertyDetailCustomization
{
public:
    bool CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly = false) override;

private:
    bool DrawIVec3Control(const std::string& label, glm::ivec3& values, int resetValue = 0, float columnWidth = 100.0f);
};