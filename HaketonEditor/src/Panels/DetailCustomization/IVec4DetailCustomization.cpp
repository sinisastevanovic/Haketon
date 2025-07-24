#include "IVec4DetailCustomization.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

bool IVec4PropertyDetailCustomization::CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly)
{
    if(bReadOnly)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);  
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
    }

    glm::ivec4 value = Value.get_value<glm::ivec4>();
    
    bool valueChanged = DrawIVec4Control("##", value);
    
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

bool IVec4PropertyDetailCustomization::DrawIVec4Control(const std::string& label, glm::ivec4& values, int resetValue, float columnWidth)
{
    ImGuiIO& io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];
    
    bool valueChanged = false;

    ImGui::PushID((label + "1").c_str());
    
    ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
    
    float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    // X component
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.7f, 0.13f, 0.09f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.93f, 0.18f, 0.0f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.6f, 0.07f, 0.11f, 1.0f});
    ImGui::PushFont(boldFont);
    if(ImGui::Button("X", buttonSize))
    {
        values.x = resetValue;
        valueChanged = true;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 4});
    ImGui::SameLine();
    valueChanged |= ImGui::DragInt("##X", &values.x, 1.0f);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
    ImGui::SameLine();

    // Y component
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.37f, 0.6f, 0.15f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.43f, 0.7f, 0.18f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.31f, 0.5f, 0.16f, 1.0f});
    ImGui::PushFont(boldFont);
    if(ImGui::Button("Y", buttonSize))
    {
        values.y = resetValue;
        valueChanged = true;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopFont();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 4});
    ImGui::SameLine();
    valueChanged |= ImGui::DragInt("##Y", &values.y, 1.0f);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
    ImGui::SameLine();

    // Z component
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.13f, 0.36f, 0.7f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.15f, 0.42f, 0.8f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.11f, 0.31f, 0.6f, 1.0f});
    ImGui::PushFont(boldFont);
    if(ImGui::Button("Z", buttonSize))
    {
        values.z = resetValue;
        valueChanged = true;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopFont();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 4});
    ImGui::SameLine();
    valueChanged |= ImGui::DragInt("##Z", &values.z, 1.0f);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
    ImGui::SameLine();

    // W component
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.7f, 0.36f, 0.13f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.8f, 0.42f, 0.15f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.6f, 0.31f, 0.11f, 1.0f});
    ImGui::PushFont(boldFont);
    if(ImGui::Button("W", buttonSize))
    {
        values.w = resetValue;
        valueChanged = true;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopFont();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 4});
    ImGui::SameLine();
    valueChanged |= ImGui::DragInt("##W", &values.w, 1.0f);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
    
    ImGui::PopID();

    return valueChanged;
}