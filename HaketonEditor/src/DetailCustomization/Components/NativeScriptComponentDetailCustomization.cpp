#include "NativeScriptComponentDetailCustomization.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <imgui_internal.h>

namespace Haketon
{
    void NativeScriptComponentDetailCustomization::CustomizeContent(rttr::instance Instance)
    {
        if(!Instance.is_valid())
            return;
    
        Haketon::NativeScriptComponent* component = Instance.try_convert<Haketon::NativeScriptComponent>();
        if(component)
        {
            ImGui::Text("Script Class");
            ImGui::SameLine();
        
            if (DrawScriptSelector(component->ScriptClassName, *component))
            {
                component->UpdateBinding();
            }

            ImGui::Separator();
            
            if (!component->Data.empty())
            {
                ImGui::Text("Script Data");
                DrawDataProperties(*component);
            }
        }
    }

    bool NativeScriptComponentDetailCustomization::DrawScriptSelector(std::string& scriptClassName, Haketon::NativeScriptComponent& component)
    {
        const auto& scripts = Haketon::ScriptRegistry::Get().GetRegisteredScripts();
    
        int currentItem = -1;
        std::vector<const char*> scriptNames;
        scriptNames.push_back("None");
    
        for (size_t i = 0; i < scripts.size(); ++i)
        {
            scriptNames.push_back(scripts[i].Name.c_str());
            if (scripts[i].Name == scriptClassName)
            {
                currentItem = static_cast<int>(i + 1);
            }
        }
    
        if (currentItem == -1)
        {
            currentItem = 0;
        }

        bool changed = false;
        if (ImGui::Combo("##ScriptClass", &currentItem, scriptNames.data(), static_cast<int>(scriptNames.size())))
        {
            if (currentItem == 0)
            {
                scriptClassName = "";
            }
            else
            {
                scriptClassName = scripts[currentItem - 1].Name;
            }
            changed = true;
        }

        return changed;
    }

    void NativeScriptComponentDetailCustomization::DrawDataProperties(NativeScriptComponent& component)
    {
        if (component.Data.empty())
            return;

        if (ImGui::BeginTable("ScriptDataTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner))
        {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < component.Data.size(); ++i)
            {
                ImGui::PushID(static_cast<int>(i));
                
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                
                // Display the name as read-only text
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%s", component.Data[i].Name.c_str());
                
                ImGui::TableSetColumnIndex(1);
                
                // Draw the value widget
                if (DrawScriptDataWidget(component.Data[i]))
                {
                    component.OnPropertyChanged("Data");
                }
                
                ImGui::PopID();
            }
            
            ImGui::EndTable();
        }
    }

    bool NativeScriptComponentDetailCustomization::DrawScriptDataWidget(ScriptData& scriptData)
    {
        return CreateValueWidgetFromVariant(scriptData.Value, scriptData.Type);
    }

    bool NativeScriptComponentDetailCustomization::CreateValueWidgetFromVariant(rttr::variant& Value, const std::string& Type, bool bReadOnly)
    {
        if (bReadOnly)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);  
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
        }
        
        const char* Label = "##";
        bool bValueChanged = false;

        // Handle different types based on the Type string
        if (Type == "bool")
        {
            bool value = Value.can_convert<bool>() ? Value.get_value<bool>() : false;
            if (ImGui::Checkbox(Label, &value) && !bReadOnly)
            {
                bValueChanged = true;
                Value = value;
            }
        }
        else if (Type == "int" || Type == "int32_t")
        {
            int value = Value.can_convert<int>() ? Value.get_value<int>() : 0;
            if (ImGui::DragInt(Label, &value) && !bReadOnly)
            {
                bValueChanged = true;
                Value = value;
            }
        }
        else if (Type == "float")
        {
            float value = Value.can_convert<float>() ? Value.get_value<float>() : 0.0f;
            if (ImGui::DragFloat(Label, &value) && !bReadOnly)
            {
                bValueChanged = true;
                Value = value;
            }
        }
        else if (Type == "double")
        {
            double value = Value.can_convert<double>() ? Value.get_value<double>() : 0.0;
            float floatValue = static_cast<float>(value);
            if (ImGui::DragFloat(Label, &floatValue) && !bReadOnly)
            {
                bValueChanged = true;
                Value = static_cast<double>(floatValue);
            }
        }
        else if (Type == "std::string" || Type.find("string") != std::string::npos)
        {
            std::string strValue = Value.can_convert<std::string>() ? Value.get_value<std::string>() : "";
            
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), strValue.c_str());           
            if (ImGui::InputText(Label, buffer, sizeof(buffer)) && !bReadOnly)
            {
                bValueChanged = true;
                Value = std::string(buffer);
            }
        }
        else if (Type == "Vector2" || Type == "glm::vec2")
        {
            glm::vec2 vec = Value.can_convert<glm::vec2>() ? Value.get_value<glm::vec2>() : glm::vec2(0.0f);
            if (ImGui::DragFloat2(Label, glm::value_ptr(vec)) && !bReadOnly)
            {
                bValueChanged = true;
                Value = vec;
            }
        }
        else if (Type == "Vector3" || Type == "glm::vec3")
        {
            glm::vec3 vec = Value.can_convert<glm::vec3>() ? Value.get_value<glm::vec3>() : glm::vec3(0.0f);
            if (ImGui::DragFloat3(Label, glm::value_ptr(vec)) && !bReadOnly)
            {
                bValueChanged = true;
                Value = vec;
            }
        }
        else if (Type == "Vector4" || Type == "glm::vec4")
        {
            glm::vec4 vec = Value.can_convert<glm::vec4>() ? Value.get_value<glm::vec4>() : glm::vec4(0.0f);
            if (ImGui::DragFloat4(Label, glm::value_ptr(vec)) && !bReadOnly)
            {
                bValueChanged = true;
                Value = vec;
            }
        }
        else
        {
            // Fallback for unknown types - display as read-only text
            std::string valueStr = Value.to_string();
            ImGui::Text("Unsupported type '%s': %s", Type.c_str(), valueStr.c_str());
        }

        if (bReadOnly)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }

        return bValueChanged;
    }
}
