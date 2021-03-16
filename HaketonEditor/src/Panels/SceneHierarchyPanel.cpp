#include "SceneHierarchyPanel.h"

#include <string>

#include "Haketon/Scene/Scene.h"
#include "Haketon/Scene/Components.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_internal.h"
#include "rttr/enumeration.h"



// TODO: Use factories like UE to register custom DetailCustomization

namespace Haketon
{
    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
    {
        SetContext(scene);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
    {
        m_Context = context;
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");

        m_Context->m_Registry.each([&](auto entityID)
        {
           Entity entity = { entityID, m_Context.get() };
           DrawEntityNode(entity);            
        });

        if(ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
        {
            m_SelectedEntity = {};
        }

        // right click on blank space
        if(ImGui::BeginPopupContextWindow(0, 1, false))
        {
            if(ImGui::MenuItem("Create empty Entity"))
                m_Context->CreateEntity("Empty Entity");

            ImGui::EndPopup();
        }
        
        ImGui::End();

        // TODO: refactor this
        ImGui::Begin("Properties");

        if(m_SelectedEntity)
        {
            DrawComponents(m_SelectedEntity);
        }
        
        ImGui::End();

        //ImGui::ShowDemoWindow();
    }

    // TODO: refactor this
    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        // TODO: Hide arrow when entity has no childs
        ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        
        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
        if(ImGui::IsItemClicked())
        {
            // TODO: Selection changed callback
            m_SelectedEntity = entity;
        }

        bool entityDeleted = false;
        if(ImGui::BeginPopupContextItem())
        {
            if(ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;
            ImGui::EndPopup();
        }

        if(opened)
        {
            ImGui::TreePop();
        }

        if(entityDeleted)
        {
            if(m_SelectedEntity == entity)
                m_SelectedEntity = {};
            
            m_Context->DestroyEntity(entity);
        }
    }
    
    static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];
        
        bool valueChanged = false;

        ImGui::PushID((label + "1").c_str());
        
        // TODO: Add copy to clipboard functionality!

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

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

        ImGui::SameLine();
        valueChanged |= ImGui::DragFloat("##X", &values.x, 0.1f); // TODO: enable edit mode with single click...
        ImGui::PopItemWidth();
        ImGui::SameLine();

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

        ImGui::SameLine();
        valueChanged |= ImGui::DragFloat("##Y", &values.y, 0.1f);
        ImGui::PopItemWidth();
        ImGui::SameLine();

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

        ImGui::SameLine();
        valueChanged |= ImGui::DragFloat("##Z", &values.z, 0.1f);
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        
        ImGui::PopID();

        return valueChanged;
    }

    static bool CanPropertyBeEdited(rttr::property& prop, rttr::instance& component)
    {
        auto EditConditionMetaData = prop.get_metadata("EditCondition");
        if(EditConditionMetaData)
        {
            /*const char* NOTToken = "!";
            size_t NOTTokenLength = strlen(NOTToken);
            const char* ANDToken = "&&";
            size_t ANDTokenLength = strlen(ANDToken);
            const char* ORToken = "||";
            size_t ORTokenLength = strlen(ORToken);*/
            bool bBoolNegate = false;
            bool bEnumNegate = false;
            bool bShouldBeEnum = false;
            
            std::string EditCondition = EditConditionMetaData.get_value<std::string>(); /* !TestBool, EnumProp == EnumVal, EnumProp != EnumVal */
            EditCondition.erase(std::remove_if(EditCondition.begin(), EditCondition.end(), isspace), EditCondition.end()); // Remove white spaces
            std::string PropertyName = EditCondition; // TODO:

            if(EditCondition.rfind("!", 0) == 0)
            {
                bBoolNegate = true;
                PropertyName = EditCondition.substr(1);                
            }

            size_t tokenPos = EditCondition.find("==");
            if(tokenPos != std::string::npos)
            {
                PropertyName = EditCondition.substr(0, tokenPos);
                bShouldBeEnum = true;
            }
            else
            {
                tokenPos = EditCondition.find("!=");
                if(tokenPos != std::string::npos)
                {
                    PropertyName = EditCondition.substr(0, tokenPos);
                    bEnumNegate = true;
                    bShouldBeEnum = true;
                }
            }
                        
            auto ConditionProp = component.get_type().get_property(PropertyName);
            HK_CORE_ASSERT(ConditionProp, "Metadata EditCondition invalid! Property {0} does not exist!", PropertyName);

            auto ConditionVal = ConditionProp.get_value(component);

            if(ConditionProp.is_enumeration())
            {
                HK_CORE_ASSERT(!bBoolNegate, "Metadata EditCondition invalid! Property {0} is an enumeration, but Condition starts with Negate!", PropertyName);
                
                auto Enum = ConditionProp.get_enumeration();
                HK_CORE_ASSERT(tokenPos != std::string::npos, "Metadata EditCondition invalid! Property {0} is an enumeration, but Condition is invalid!", PropertyName);

                size_t begin = tokenPos + 2;
                std::string CompareValueStr = EditCondition.substr(begin);
                std::string PropValueStr = ConditionVal.to_string();

                return bEnumNegate ? PropValueStr != CompareValueStr : PropValueStr == CompareValueStr;
            }

            HK_CORE_ASSERT(!bShouldBeEnum, "Metadata EditCondition invalid! Property {0} is not an enum, but given condition is only supported with enumerations", PropertyName);

            HK_CORE_ASSERT(ConditionVal.is_type<bool>(), "Metadata EditCondition invalid! Property {0} is not of type bool or enum", PropertyName);

            bool Result = ConditionVal.get_value<bool>();
            if(bBoolNegate)
                Result = !Result;
            
            return Result;

        }
        
        return true;
    }

    static void CreatePropertySection(rttr::property& prop, rttr::instance& component)
    {
        if(prop.get_metadata("HideInDetails") ? true : false)
            return;
            
        bool bDisabled = !CanPropertyBeEdited(prop, component);

        if(bDisabled)
        {
            if(prop.get_metadata("HideWhenDisabled") ? true : false)
                return;
            
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);  
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
        }
        
        rttr::variant value = prop.get_value(component);
        
        rttr::type type = value.get_type().get_raw_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();
        rttr::instance inst(value);

        rttr::instance obj = inst.get_type().get_raw_type().is_wrapper() ? inst.get_wrapped_instance() : inst;

        uint32_t numProps = type.get_properties().size();
        if(numProps > 0)
        {
            for(auto subprop : type.get_properties())
            {
                 CreatePropertySection(subprop, obj);
            }
        }
        else
        {
            
            std::string propertyName = prop.get_name().to_string();
            auto propValue = prop.get_value(component);
            auto label = "##" + propertyName;

            bool convertToDegrees = prop.get_metadata("Degrees") ? true : false;



            // TODO: Add copy to clipboard functionality!
            ImGui::Text(propertyName.c_str());
            
            ImGui::NextColumn();

            if(prop.is_enumeration())
            {
                rttr::enumeration enumeration = prop.get_enumeration();
                auto names = enumeration.get_names();
                std::string CurrentValueString = propValue.to_string();

                if (ImGui::BeginCombo(label.c_str(), CurrentValueString.c_str()))
                {
                    for (auto name : names)
                    {
                        std::string namechar = name.to_string();
                        const bool is_selected = (namechar == CurrentValueString);
                        if(ImGui::Selectable(namechar.c_str(), is_selected))
                            prop.set_value(component, enumeration.name_to_value(namechar));

                        if(is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
       
                    ImGui::EndCombo();
                }
            }
            else if(propValue.is_type<int>())
            {
                int value = propValue.get_value<int>();
                if(ImGui::DragInt(label.c_str(), &value))
                    prop.set_value(component, value);
            }
            else if(propValue.is_type<float>())
            {
               // prop.set_value(component, 345.0f);
                //propValue = prop.get_value(component);
                float value = convertToDegrees ? glm::degrees(propValue.get_value<float>()) : propValue.get_value<float>();
                
                if(ImGui::DragFloat(label.c_str(), &value))
                    prop.set_value(component, convertToDegrees ? glm::radians(value) : value);
            }
            else if(propValue.is_type<bool>())
            {
                bool value = propValue.get_value<bool>();
                if(ImGui::Checkbox(label.c_str(), &value))
                    prop.set_value(component, value);
            }
            else if(propValue.is_type<std::string>())
            {
                std::string value = propValue.get_value<std::string>();

                char buffer[256];
                memset(buffer, 0, sizeof(buffer));
                strcpy_s(buffer, sizeof(buffer), value.c_str());           
                if(ImGui::InputText(label.c_str(), buffer, sizeof(buffer)))
                    prop.set_value(component, std::string(buffer));
            }
            else if(propValue.is_type<glm::vec3>())
            {
                glm::vec3 value = convertToDegrees ? glm::degrees(propValue.get_value<glm::vec3>()) : propValue.get_value<glm::vec3>();
                
                if(DrawVec3Control(label, value))
                    prop.set_value(component, convertToDegrees ? glm::radians(value) : value);          
            }
            else if(propValue.is_type<glm::vec4>())
            {
                glm::vec4 value = propValue.get_value<glm::vec4>();
                if(ImGui::ColorEdit4(label.c_str(), glm::value_ptr(value)))
                    prop.set_value(component, value);
            }

            ImGui::NextColumn();

        }

        if(bDisabled)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }

    }

    template<typename T>
    static void CreateComponentSection(Entity entity, bool isRemovable = true, std::function<void(const T&)> uiFunction = nullptr)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

        if(entity.HasComponent<T>())
        {
            auto& component = entity.GetComponent<T>();
            rttr::type t = rttr::type::get(component);
            rttr::instance compInstance(component);

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, t.get_name().to_string().c_str());
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
                
                ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - lineHeight * 0.5f);
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
                static bool ColumnsInitialized = false;

                uint32_t numProps = t.get_properties().size();
                if(numProps > 0)
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));

                ImGui::Columns(2, "myColumns");
                if(!ColumnsInitialized)
                {
                    ImGui::SetColumnWidth(0, 100.0f);
                    ColumnsInitialized = true;
                }
                            
                uint32_t currentIndex = 0;
                for(auto prop : t.get_properties())
                {
                    CreatePropertySection(prop, compInstance);
                    ++currentIndex;                   
                }
                
                if(numProps > 0)
                    ImGui::PopStyleVar();

                if(uiFunction != nullptr)
                    uiFunction(component);
                
                ImGui::Columns(1);
                ImGui::TreePop();
            }

            if(removeComponent && isRemovable)
                entity.RemoveComponent<T>();
        }
    }

    
    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        // Use factories like UE for this...
        if(entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), tag.c_str());
            if(ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                tag = std::string(buffer);
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if(ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");

        if(ImGui::BeginPopup("AddComponent"))
        {
            if(ImGui::MenuItem("Camera"))
            {
                m_SelectedEntity.AddComponent<CameraComponent>();
                ImGui::CloseCurrentPopup();
            }

            if(ImGui::MenuItem("Sprite Renderer"))
            {
                m_SelectedEntity.AddComponent<SpriteRendererComponent>();
                ImGui::CloseCurrentPopup();
            }

            if(ImGui::MenuItem("Int Component"))
            {
                m_SelectedEntity.AddComponent<IntComponent>();
                ImGui::CloseCurrentPopup();
            }
                
            ImGui::EndPopup();
        }
        ImGui::PopItemWidth();


        CreateComponentSection<TransformComponent>(entity, false);
        /*CreateComponentSection<CameraComponent>(entity, false, [](auto& component)
        {
            auto& camera = component.Camera;
            rttr::type t = rttr::type::get(camera);
            rttr::instance inst(camera);
            for(auto cameraProp : t.get_properties())
            {
                CreatePropertySection(cameraProp, inst);
            }
        });*/

        CreateComponentSection<CameraComponent>(entity);
        CreateComponentSection<SpriteRendererComponent>(entity, true); 
        CreateComponentSection<IntComponent>(entity, true); 
    }
}
