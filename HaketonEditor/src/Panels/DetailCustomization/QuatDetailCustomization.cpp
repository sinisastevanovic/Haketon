#include "QuatDetailCustomization.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Haketon
{
    bool QuatPropertyDetailCustomization::CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly)
    {
        if(bReadOnly)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);  
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
        }

        bool showAsEuler = Property.get_metadata("Euler") ? true : false;
        glm::quat value = Value.get_value<glm::quat>();
        
        bool valueChanged = false;
        
        if(showAsEuler)
        {
            FVec3 eulerAngles = glm::degrees(glm::eulerAngles(value));
            valueChanged = DrawEulerControl("##", eulerAngles);
            
            if(valueChanged && !bReadOnly)
            {
                value = glm::quat(glm::radians(eulerAngles));
            }
        }
        else
        {
            valueChanged = DrawQuatControl("##", value);
        }
        
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

    bool QuatPropertyDetailCustomization::DrawEulerControl(const std::string& label, FVec3& eulerAngles)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];
        
        bool valueChanged = false;

        ImGui::PushID((label + "euler").c_str());
        
        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        
        float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        // X component (Pitch)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.7f, 0.13f, 0.09f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.93f, 0.18f, 0.0f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.6f, 0.07f, 0.11f, 1.0f});
        ImGui::PushFont(boldFont);
        if(ImGui::Button("X", buttonSize))
        {
            eulerAngles.x = 0.0f;
            valueChanged = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);
        
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 4});
        ImGui::SameLine();
        valueChanged |= ImGui::DragFloat("##X", &eulerAngles.x, 0.1f);
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();
        ImGui::SameLine();

        // Y component (Yaw)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.37f, 0.6f, 0.15f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.43f, 0.7f, 0.18f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.31f, 0.5f, 0.16f, 1.0f});
        ImGui::PushFont(boldFont);
        if(ImGui::Button("Y", buttonSize))
        {
            eulerAngles.y = 0.0f;
            valueChanged = true;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopFont();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 4});
        ImGui::SameLine();
        valueChanged |= ImGui::DragFloat("##Y", &eulerAngles.y, 0.1f);
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();
        ImGui::SameLine();

        // Z component (Roll)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.13f, 0.36f, 0.7f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.15f, 0.42f, 0.8f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.11f, 0.31f, 0.6f, 1.0f});
        ImGui::PushFont(boldFont);
        if(ImGui::Button("Z", buttonSize))
        {
            eulerAngles.z = 0.0f;
            valueChanged = true;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopFont();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 4});
        ImGui::SameLine();
        valueChanged |= ImGui::DragFloat("##Z", &eulerAngles.z, 0.1f);
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();
        
        ImGui::PopID();

        return valueChanged;
    }

    bool QuatPropertyDetailCustomization::DrawQuatControl(const std::string& label, glm::quat& values, float resetValue, float columnWidth)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];
        
        bool valueChanged = false;

        ImGui::PushID((label + "quat").c_str());
        
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
        valueChanged |= ImGui::DragFloat("##X", &values.x, 0.01f);
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
        valueChanged |= ImGui::DragFloat("##Y", &values.y, 0.01f);
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
        valueChanged |= ImGui::DragFloat("##Z", &values.z, 0.01f);
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
        valueChanged |= ImGui::DragFloat("##W", &values.w, 0.01f);
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();
        
        ImGui::PopID();

        return valueChanged;
    }
}