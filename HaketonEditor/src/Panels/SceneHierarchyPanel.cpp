#include "SceneHierarchyPanel.h"

#include <string>

#include "Haketon/Scene/Scene.h"
#include "Haketon/Scene/Components.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "PropertyEditorModule.h"
#include "DetailCustomization/IDetailCustomization.h"
#include "Haketon/Core/ModuleManager.h"
#include "Haketon/Scene/Components/CameraComponent.h"
#include "Haketon/Scene/Components/TagComponent.h"
#include "imgui/imgui_internal.h"
#include "rttr/enumeration.h"



// TODO: Use factories like UE to register custom DetailCustomization

namespace Haketon
{

    float SceneHierarchyPanel::minRowHeight = 30.0f;
    int SceneHierarchyPanel::CurrentIndentation = 0;

    /*----------------- NEW STUFF ------------------*/

    // TODO: Refactor this to Control/Widget Library
    static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];
        
        bool valueChanged = false;

        ImGui::PushID((label + "1").c_str());
        
        // TODO: Add copy to clipboard functionality!

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

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
        
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 4});
        ImGui::SameLine();
        valueChanged |= ImGui::DragFloat("##X", &values.x, 0.1f); // TODO: enable edit mode with single click...
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();
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

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 4});
        ImGui::SameLine();
        valueChanged |= ImGui::DragFloat("##Y", &values.y, 0.1f);
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();
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

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 4});
        ImGui::SameLine();
        valueChanged |= ImGui::DragFloat("##Z", &values.z, 0.1f);
        ImGui::PopStyleVar();
        ImGui::PopItemWidth();
        
        ImGui::PopID();

        return valueChanged;
    }

    
    bool CreateLabelWidget(const char* Name, const char* ToolTip, bool bExpandable = false)
    {
        ImGuiTreeNodeFlags TreeNodeFlags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Framed;
        if(!bExpandable)
        {          
            TreeNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;            
        }

        const char* fmt = bExpandable ? "%s" : "\t%s";
        bool Open = ImGui::TreeNodeEx(Name, TreeNodeFlags, fmt, Name);        
        
        return bExpandable && Open;
    }
    
    bool CreatePropertyNameWidget(rttr::property& Property, const char* NameOverride)
    {
        rttr::type PropType = Property.get_type().get_raw_type().is_wrapper() ? Property.get_type().get_wrapped_type() : Property.get_type();
        uint32_t numProps = PropType.get_properties().size();
        bool bExpandable = numProps > 0 || PropType.is_sequential_container();
        
        if(bExpandable && (PropType == rttr::type::get<glm::vec3>() || PropType == rttr::type::get<glm::vec4>()))
        {
            bExpandable = false;
        }
        
        return CreateLabelWidget(NameOverride, "", bExpandable);
    }
    
    bool CreatePropertyNameWidget(rttr::property& Property)
    {
        return CreatePropertyNameWidget(Property, Property.get_name().to_string().c_str());
    }

    bool CreateValueWidget(rttr::variant& Value, rttr::property& ParentProperty)
    {
        rttr::type ValueType = Value.get_type();
        const char* Label = "##";

        bool bValueChanged = false;
        
        if(ValueType.is_arithmetic())
        {
            if (ValueType == rttr::type::get<bool>())
            {
                bool value = Value.get_value<bool>();
                if(ImGui::Checkbox(Label, &value))
                {
                    bValueChanged = true;
                    Value = value;
                }
            }
            else if (ValueType == rttr::type::get<char>())
            {
                std::string value(1, Value.get_value<char>());
                char buffer[256];
            
                memset(buffer, 0, sizeof(buffer));
                strcpy_s(buffer, sizeof(buffer), value.c_str());
                if(ImGui::InputText(Label, buffer, sizeof(buffer)))
                {
                    bValueChanged = true;
                    Value = ((std::string)buffer)[0];
                }
            }
            else if (ValueType == rttr::type::get<int8_t>())
            {
                int32_t value = Value.get_value<int8_t>();
                if(ImGui::DragInt(Label, &value, 1, std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max()))
                {
                    bValueChanged = true;
                    Value = (int8_t)value;
                }
            }
            else if (ValueType == rttr::type::get<int16_t>())
            {
                int32_t value = Value.get_value<int16_t>();
                if(ImGui::DragInt(Label, &value, 1, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max()))
                {
                    bValueChanged = true;
                    Value = (int16_t)value;
                }
            }
            else if (ValueType == rttr::type::get<int32_t>())
            {
                int32_t value = Value.get_value<int32_t>();
                if(ImGui::DragInt(Label, &value, 1, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()))
                {
                    bValueChanged = true;
                    Value = value;
                }
            }
            else if (ValueType == rttr::type::get<float>())
            {              
                bool convertToDegrees = ParentProperty.get_metadata("Degrees") ? true : false;
                float value = convertToDegrees ? glm::degrees(Value.get_value<float>()) : Value.get_value<float>();                
                if(ImGui::DragFloat(Label, &value))
                {
                    bValueChanged = true;
                    Value = convertToDegrees ? glm::radians(value) : value;
                }                   
            }
            else if (ValueType == rttr::type::get<double>())
            {              
                bool convertToDegrees = ParentProperty.get_metadata("Degrees") ? true : false;
                float value = convertToDegrees ? glm::degrees(Value.get_value<double>()) : Value.get_value<double>();                
                if(ImGui::DragFloat(Label, &value))
                {
                    bValueChanged = true;
                    Value = (double)(convertToDegrees ? glm::radians(value) : value);
                }                   
            }
        }
        else if(ValueType == rttr::type::get<std::string>())
        {        
            std::string strValue = Value.get_value<std::string>();

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), strValue.c_str());           
            if(ImGui::InputText(Label, buffer, sizeof(buffer)))
            {
                bValueChanged = true;
                Value = std::string(buffer);
            }
        }
        else if(ValueType.is_enumeration())
        {      
            rttr::enumeration enumeration = ValueType.get_enumeration();
            auto names = enumeration.get_names();
            std::string CurrentValueString = Value.to_string();
    
            if (ImGui::BeginCombo(Label, CurrentValueString.c_str()))
            {
                for (auto name : names)
                {
                    std::string namechar = name.to_string();
                    const bool is_selected = (namechar == CurrentValueString);
                    if(ImGui::Selectable(namechar.c_str(), is_selected))
                    {
                        bValueChanged = true;
                        Value = enumeration.name_to_value(namechar);                        
                    }
    
                    if(is_selected)
                        ImGui::SetItemDefaultFocus();
                }
    
                ImGui::EndCombo();
            }
        }       
        else if(ValueType == rttr::type::get<glm::vec3>())
        {
            bool convertToDegrees = ParentProperty.get_metadata("Degrees") ? true : false;
            glm::vec3 value = convertToDegrees ? glm::degrees(Value.get_value<glm::vec3>()) : Value.get_value<glm::vec3>();
            
            if(DrawVec3Control(Label, value))
            {
                bValueChanged = true;
                Value = convertToDegrees ? glm::radians(value) : value;
            }
        }
        else if(ValueType == rttr::type::get<glm::vec4>())
        {               
            glm::vec4 value = Value.get_value<glm::vec4>();
            if(ImGui::ColorEdit4(Label, glm::value_ptr(value)))
            {
                bValueChanged = true;
                Value = value;
            }
        }

        return bValueChanged;
    }

    /*----------------- NEW STUFF END ------------------*/
    
    
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

        ImGui::ShowDemoWindow();
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

        std::string propName = prop.get_name().to_string();
        ImGui::PushID(propName.c_str());

        ImGui::TableNextRow(0, SceneHierarchyPanel::minRowHeight);
        ImGui::TableSetColumnIndex(0);

        bool bNameWidgetOpen;

        ImGui::Indent(SceneHierarchyPanel::CurrentIndentation);
        bNameWidgetOpen = CreatePropertyNameWidget(prop);
        ImGui::Unindent(SceneHierarchyPanel::CurrentIndentation); 

        
        ImGui::TableNextColumn();

        rttr::variant value = prop.get_value(component);

        auto ValueType = value.get_type().get_raw_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();
        auto WrappedType = ValueType.is_wrapper() ? ValueType.get_wrapped_type() : ValueType;
        bool bIsWrapper = WrappedType != ValueType;

        rttr::instance inst(value);

        rttr::instance obj = inst.get_type().get_raw_type().is_wrapper() ? inst.get_wrapped_instance() : inst;

        uint32_t numProps = ValueType.get_properties().size();
        if(numProps > 0 && !value.is_type<glm::vec4>() && !value.is_type<glm::vec3>())
        {
            if(bNameWidgetOpen)
            {            
                SceneHierarchyPanel::CurrentIndentation += 20.0f;
                for(auto subprop : ValueType.get_properties())
                {
                    CreatePropertySection(subprop, obj); // TODO: Think about if these 'sub-properties' should be shown in another category...
                }
                SceneHierarchyPanel::CurrentIndentation -= 20.0f;
               
                ImGui::TreePop();
            }
        }
        else if(value.is_sequential_container()) // TODO: Add and Clear Button. Remove, Insert.
        {
            auto View = value.create_sequential_view();
            int NumItems = View.get_size();

            bool bPropertyChanged = false;

            ImGui::Text("%d Array elements", NumItems);
            
            if(bNameWidgetOpen)
            {
                ImGui::Indent(20.0f);
                for(int i = 0; i < NumItems; i++)
                {
                    std::string indexAsString = std::to_string(i);
                    propName += indexAsString;
                    
                    ImGui::PushID(propName.c_str());
                    
                    ImGui::TableNextColumn();
                    CreateLabelWidget(indexAsString.c_str(), "");

                    ImGui::TableNextColumn();
                    auto ItemValue = View.get_value(i);
                    auto WrappedVar = View.get_value(i).extract_wrapped_value();
                    if(CreateValueWidget(WrappedVar, prop))
                    {                    
                        bPropertyChanged = true;
                        View.set_value(i, WrappedVar);
                    }

                    ImGui::PopID();
                }
                ImGui::Unindent(20.0f);

                if(bPropertyChanged)
                    prop.set_value(component, value);
                
                ImGui::TreePop();
            }
        }
        else if(value.is_associative_container()) // TODO: Implement assiciative container
        {
            ImGui::Text("Associative Container not supported yet");
        }
        else
        {
            if(CreateValueWidget(value, prop))
                prop.set_value(component, value);
        }

        ImGui::PopID();
        
        if(bDisabled)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }       
    }

    template<typename T>
    static void CreateComponentSection(Entity entity, bool isRemovable = true)
    {
        // TODO: Should you see all components details at once? Or does the user have to select components like in UE...
        
        if(entity.HasComponent<T>())
        {
            ImGui::PushID((void*)typeid(T).hash_code());
            
            auto& component = entity.GetComponent<T>();
            rttr::type t = rttr::type::get(component);
            rttr::instance compInstance(component);

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
                float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
                
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
                    static ImGuiTableFlags TableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings;
                    float InnerWidth = 0.0f;
                    if(ImGui::BeginTable("PropertyTable", 2, TableFlags))
                    {
                        ImGui::TableSetupColumn("Name",		ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch, 0.5f);
                        ImGui::TableSetupColumn("Value",	ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch, 0.5f);
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
 
    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        CreateComponentSection<TagComponent>(entity, false);

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
                
            ImGui::EndPopup();
        }
        ImGui::PopItemWidth();


        CreateComponentSection<TransformComponent>(entity, false);
        CreateComponentSection<CameraComponent>(entity);
        CreateComponentSection<SpriteRendererComponent>(entity, true); 
    }
}
