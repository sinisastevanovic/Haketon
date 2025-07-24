#include "ColorDetailCustomization.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

bool ColorPropertyDetailCustomization::CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly)
{
    if(bReadOnly)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);  
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
    }

    Haketon::FColor value = Value.get_value<Haketon::FColor>();
    
    bool valueChanged = DrawColorControl("##", value);
    
    if(valueChanged && !bReadOnly)
    {
        Value = value;
    }

    if(bReadOnly)
    {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }

    return valueChanged && !bReadOnly;
}

bool ColorPropertyDetailCustomization::DrawColorControl(const std::string& label, Haketon::FColor& color)
{
    return ImGui::ColorEdit4(label.c_str(), glm::value_ptr(color.rgba));
}