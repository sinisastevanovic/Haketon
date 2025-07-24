#pragma once

#include "Haketon/Scene/Entity.h"
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include "Modules/PropertyEditorModule.h"
#include "DetailCustomization/IDetailCustomization.h"
#include "Haketon/Core/ModuleManager.h"
#include "imgui/imgui_internal.h"
#include "rttr/enumeration.h"

namespace Haketon
{
    // Forward declaration for function defined in SceneHierarchyPanel.cpp
    void CreatePropertySection(rttr::property& prop, rttr::instance& component);
    
    // Template function to create component sections
    template<typename T>
    void CreateComponentSection(Entity entity, bool isRemovable = true)
    {
        // TODO: Should you see all components details at once? Or does the user have to select components like in UE...
        if(entity.HasComponent<T>())
        {
            //ImGui::PushID((void*)typeid(T).hash_code());
            
            auto& component = entity.GetComponent<T>();
            rttr::type t = rttr::type::get(component);
            rttr::instance compInstance(component);

            ImGui::PushID(t.get_name().to_string().c_str());

            PropertyEditorModule* PropertyEditor = ModuleManager::LoadModuleChecked<PropertyEditorModule>("PropertyEditor");
            Ref<IDetailCustomization> DetailCustomization = PropertyEditor->GetDetailCustomization(t.get_name().to_string());
            if(DetailCustomization)
            {
                DetailCustomization->CustomizeDetails(compInstance);
            }
            else
            {
                ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
                float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
                
                ImGui::Separator();
                
                bool open = ImGui::CollapsingHeader(t.get_name().to_string().c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);

                ImGui::PopStyleVar();
    
                bool removeComponent = false;
                                      
                if(isRemovable) // TODO: Custom menu actions for components
                {
                    if(ImGui::BeginPopupContextItem())
                    {
                        if(ImGui::MenuItem("Remove Component"))
                            removeComponent = true;
                        ImGui::EndPopup();
                    }
                    
                    ImGui::SameLine((ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x) - lineHeight * 0.5f);
                    if(ImGui::Button("...", ImVec2{lineHeight, lineHeight}))
                        ImGui::OpenPopup("ComponentSettings");
            
                    if(ImGui::BeginPopup("ComponentSettings"))
                    {
                        if(ImGui::MenuItem("Remove component"))
                            removeComponent = true;
                    
                        ImGui::EndPopup();
                    }
                }
    
                if(open)
                {
                    for(auto Method : t.get_methods())
                    {
                        if(Method.get_metadata("CallInEditor") ? true : false)
                        {
                            if(ImGui::Button(Method.get_name().to_string().c_str()))
                            {
                                Method.invoke(compInstance);
                            }
                        }                     
                    }
                   
                    static ImGuiTableFlags TableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings;
                    float InnerWidth = 0.0f;
                    if(ImGui::BeginTable("PropertyTable", 2, TableFlags, ImVec2(0, 0), InnerWidth))
                    {
                        ImGui::TableSetupColumn("Name",		ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch, 0.25f);
                        ImGui::TableSetupColumn("Value",	ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch, 0.75f);
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{0, 0, 0, 0});
                        
                        for(auto prop : t.get_properties())
                        {
                            CreatePropertySection(prop, compInstance);                            
                        }
                        
                        ImGui::PopStyleColor();
                        ImGui::EndTable();
                    }                 
                }
                ImGui::PopStyleVar();
                if(removeComponent && isRemovable)
                    entity.RemoveComponent<T>();
            }

            ImGui::PopID();
        }
    }
}