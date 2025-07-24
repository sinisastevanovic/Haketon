#pragma once
#include "IDetailCustomization.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class QuatPropertyDetailCustomization : public IPropertyDetailCustomization
{
public:
    bool CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly = false) override;

private:
    bool DrawQuatControl(const std::string& label, glm::quat& values, float resetValue = 0.0f, float columnWidth = 100.0f);
    bool DrawEulerControl(const std::string& label, glm::vec3& eulerAngles);
};