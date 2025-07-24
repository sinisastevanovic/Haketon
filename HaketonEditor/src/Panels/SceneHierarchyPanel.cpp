#include "SceneHierarchyPanel.h"
#include "ComponentRegistry.h"
#include "ComponentSectionHelper.h"
#include "EditConditionEvaluator.h"

#include <string>

#include "Haketon/Scene/Scene.h"
#include "Haketon/Scene/Components.h"
#include "Scene/CustomTestComponent.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/include/GLFW/glfw3.h>

#include "PropertyEditorModule.h"
#include "DetailCustomization/IDetailCustomization.h"
#include "Haketon/Core/ModuleManager.h"
#include "Haketon/Core/Serializer.h"
#include "Haketon/Scene/Components/CameraComponent.h"
#include "Haketon/Scene/Components/TagComponent.h"
#include "imgui/imgui_internal.h"
#include "rttr/enumeration.h"


// TODO: Use factories like UE to register custom DetailCustomization

namespace Haketon
{
    float SceneHierarchyPanel::minRowHeight = 30.0f;
    int SceneHierarchyPanel::CurrentIndentation = 0;
    bool SceneHierarchyPanel::bAddVarsInititalized = false;

    /*----------------- NEW STUFF ------------------*/

    static rttr::variant CreateDefaultVarFromType(const rttr::type& Type)
    {
        rttr::variant result = Type.create();

        if(result.is_valid())
            return result;
        else
        {
            if(Type.is_arithmetic())
            {
                if (Type == rttr::type::get<bool>())
                    result = false;
                else if(Type == rttr::type::get<char>())
                    result = '\0';
                else if (Type == rttr::type::get<int8_t>() || Type == rttr::type::get<uint8_t>() || Type == rttr::type::get<int16_t>() || Type == rttr::type::get<uint16_t>()
                     || Type == rttr::type::get<int32_t>() || Type == rttr::type::get<uint32_t>() || Type == rttr::type::get<int64_t>() || Type == rttr::type::get<uint64_t>())
                    result = 0;
                else if(Type == rttr::type::get<float>() || Type == rttr::type::get<double>())
                    result = 0.0f;              
            }
            else if(Type == rttr::type::get<std::string>())
                result = "";
        }

        result.convert(Type);
        return result;
    }

    // TODO: Refactor this to Control/Widget Library
    static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];
        
        bool valueChanged = false;

        ImGui::PushID((label + "1").c_str());
        
        // TODO: Add copy to clipboard functionality!

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        
        float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
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
        if (strlen(ToolTip) > 0 && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip(ToolTip);
        }
        
        return bExpandable && Open;
    }
    
    bool CreatePropertyNameWidget(rttr::property& Property, const char* NameOverride)
    {
        rttr::type PropType = Property.get_type().get_raw_type().is_wrapper() ? Property.get_type().get_wrapped_type() : Property.get_type();
        uint32_t numProps = PropType.get_properties().size();
        bool bExpandable = numProps > 0 || PropType.is_sequential_container() || PropType.is_associative_container();
        
        if(bExpandable && (PropType == rttr::type::get<glm::vec3>() || PropType == rttr::type::get<glm::vec4>()))
        {
            bExpandable = false;
        }

        std::string ToolTip = "";
        auto ToolTipMetadata = Property.get_metadata("Tooltip");
        if (ToolTipMetadata.is_valid())
        {
            ToolTip = ToolTipMetadata.get_value<std::string>();
        }
        return CreateLabelWidget(NameOverride, ToolTip.c_str(), bExpandable);
    }
    
    bool CreatePropertyNameWidget(rttr::property& Property)
    {
        return CreatePropertyNameWidget(Property, Property.get_name().to_string().c_str());
    }

    bool CreateValueWidget(rttr::variant& Value, rttr::property& ParentProperty, bool bReadOnly = false)
    {
        if(bReadOnly)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);  
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
        }
        
        rttr::type ValueType = Value.get_type();
        const char* Label = "##";

        bool bMinMetadata = false;
        uint64_t MinValue = 0;
        auto MinMetadata = ParentProperty.get_metadata("Min");
        if(MinMetadata)
        {
            bMinMetadata = MinMetadata.can_convert<uint64_t>();
            MinValue = MinMetadata.get_value<uint64_t>();
        }
        bool bMaxMetadata = false;
        uint64_t MaxValue = 0;
        auto MaxMetadata = ParentProperty.get_metadata("Max");
        if(MaxMetadata)
        {
            bMaxMetadata = MaxMetadata.can_convert<uint64_t>();
            MaxValue = MaxMetadata.get_value<uint64_t>();
        }

        bool bValueChanged = false;       
        
        if(ValueType.is_arithmetic())
        {
            if (ValueType == rttr::type::get<bool>())
            {
                bool value = Value.get_value<bool>();
                if(ImGui::Checkbox(Label, &value) && !bReadOnly)
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
                if(ImGui::InputText(Label, buffer, sizeof(buffer)) && !bReadOnly)
                {
                    bValueChanged = true;
                    Value = ((std::string)buffer)[0];
                }
            }
            else if (ValueType == rttr::type::get<int8_t>())
            {
                int8_t value = Value.get_value<int8_t>();              
                if(ImGui::DragScalar(Label, ImGuiDataType_S8, &value, 1, bMinMetadata ? &MinValue : nullptr, bMaxMetadata ? &MaxValue : nullptr) && !bReadOnly)
                {
                    bValueChanged = true;
                    Value = value;
                }
            }
            else if (ValueType == rttr::type::get<int16_t>())
            {
                int16_t value = Value.get_value<int16_t>();              
                if(ImGui::DragScalar(Label, ImGuiDataType_S16, &value, 1, bMinMetadata ? &MinValue : nullptr, bMaxMetadata ? &MaxValue : nullptr) && !bReadOnly)
                {
                    bValueChanged = true;
                    Value = value;
                }
            }
            else if (ValueType == rttr::type::get<int32_t>())
            {
                int32_t value = Value.get_value<int32_t>();              
                if(ImGui::DragScalar(Label, ImGuiDataType_S32, &value, 1, bMinMetadata ? &MinValue : nullptr, bMaxMetadata ? &MaxValue : nullptr) && !bReadOnly)
                {
                    bValueChanged = true;
                    Value = value;
                }
            }
            else if (ValueType == rttr::type::get<int64_t>())
            {
                int64_t value = Value.get_value<int64_t>();              
                if(ImGui::DragScalar(Label, ImGuiDataType_S64, &value, 1, bMinMetadata ? &MinValue : nullptr, bMaxMetadata ? &MaxValue : nullptr) && !bReadOnly)
                {
                    bValueChanged = true;
                    Value = value;
                }
            }
            else if (ValueType == rttr::type::get<uint8_t>())
            {
                uint8_t value = Value.get_value<uint8_t>();              
                if(ImGui::DragScalar(Label, ImGuiDataType_U8, &value, 1, bMinMetadata ? &MinValue : nullptr, bMaxMetadata ? &MaxValue : nullptr) && !bReadOnly)
                {
                    bValueChanged = true;
                    Value = value;
                }
            }
            else if (ValueType == rttr::type::get<uint16_t>())
            {
                uint16_t value = Value.get_value<uint16_t>();              
                if(ImGui::DragScalar(Label, ImGuiDataType_U16, &value, 1, bMinMetadata ? &MinValue : nullptr, bMaxMetadata ? &MaxValue : nullptr) && !bReadOnly)
                {
                    bValueChanged = true;
                    Value = value;
                }
            }
            else if (ValueType == rttr::type::get<uint32_t>())
            {
                uint32_t value = Value.get_value<uint32_t>();              
                if(ImGui::DragScalar(Label, ImGuiDataType_U32, &value, 1, bMinMetadata ? &MinValue : nullptr, bMaxMetadata ? &MaxValue : nullptr) && !bReadOnly)
                {
                    bValueChanged = true;
                    Value = value;
                }
            }
            else if (ValueType == rttr::type::get<uint64_t>())
            {
                uint64_t value = Value.get_value<uint64_t>();
                if(ImGui::DragScalar(Label, ImGuiDataType_S64, &value, 1, bMinMetadata ? &MinValue : nullptr, bMaxMetadata ? &MaxValue : nullptr) && !bReadOnly)
                {
                    bValueChanged = true;
                    Value = value;
                }
            }
            else if (ValueType == rttr::type::get<float>())
            {              
                bool convertToDegrees = ParentProperty.get_metadata("Degrees") ? true : false;
                float value = convertToDegrees ? glm::degrees(Value.get_value<float>()) : Value.get_value<float>();
                if(ImGui::DragScalar(Label, ImGuiDataType_Float, &value, 1, bMinMetadata ? &MinValue : nullptr, bMaxMetadata ? &MaxValue : nullptr) && !bReadOnly)
                {
                    bValueChanged = true;
                    Value = convertToDegrees ? glm::radians(value) : value;
                }                   
            }
            else if (ValueType == rttr::type::get<double>())
            {              
                bool convertToDegrees = ParentProperty.get_metadata("Degrees") ? true : false;
                double value = convertToDegrees ? glm::degrees(Value.get_value<double>()) : Value.get_value<double>();                
                if(ImGui::DragScalar(Label, ImGuiDataType_Double, &value, 1, bMinMetadata ? &MinValue : nullptr, bMaxMetadata ? &MaxValue : nullptr) && !bReadOnly)
                {
                    bValueChanged = true;
                    Value = (convertToDegrees ? glm::radians(value) : value);
                }                   
            }
        }
        else if(ValueType == rttr::type::get<std::string>())
        {        
            std::string strValue = Value.get_value<std::string>();

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), strValue.c_str());           
            if(ImGui::InputText(Label, buffer, sizeof(buffer)) && !bReadOnly)
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
    
            if (ImGui::BeginCombo(Label, CurrentValueString.c_str()) && !bReadOnly)
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
            
            if(DrawVec3Control(Label, value) && !bReadOnly)
            {
                bValueChanged = true;
                Value = convertToDegrees ? glm::radians(value) : value;
            }
        }
        else if(ValueType == rttr::type::get<glm::vec4>())
        {               
            glm::vec4 value = Value.get_value<glm::vec4>();
            if(ImGui::ColorEdit4(Label, glm::value_ptr(value)) && !bReadOnly)
            {
                bValueChanged = true;
                Value = value;
            }
        }

        if(bReadOnly)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
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
        m_SelectedEntity = {}; // TODO: When we have multiple Viewports, save what was selected...
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
        if(ImGui::BeginPopupContextWindow(0, 1))
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
            std::string EditCondition = EditConditionMetaData.get_value<std::string>();
            return EditConditionEvaluator::EvaluateCondition(EditCondition, component);
        }
        
        return true;
    }

    void CreatePropertySection(rttr::property& prop, rttr::instance& component)
    {
        // TODO: Currently CreateValueWidget can't be used to create array etc... We want to be able to draw Value Widgets for all types everywhere
        
        if(prop.get_metadata("HideInDetails") ? true : false)
            return;

        auto VisibleConditionMetaData = prop.get_metadata("VisibleCondition");
        if(VisibleConditionMetaData)
        {
            std::string VisibleCondition = VisibleConditionMetaData.get_value<std::string>();
            if (!EditConditionEvaluator::EvaluateCondition(VisibleCondition, component))
                return;
        }
            
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
        bool bForceExpand = false;

        ImGui::Indent(SceneHierarchyPanel::CurrentIndentation);
        if(bForceExpand)
        {
            ImGui::SetNextItemOpen(true);
        }
        bNameWidgetOpen = CreatePropertyNameWidget(prop);
        ImGui::Unindent(SceneHierarchyPanel::CurrentIndentation);

        

        
        ImGui::TableNextColumn();

        rttr::variant value = prop.get_value(component);

        auto ValueType = value.get_type().get_raw_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();
        auto WrappedType = ValueType.is_wrapper() ? ValueType.get_wrapped_type() : ValueType;
        bool bIsWrapper = WrappedType != ValueType;

        rttr::instance inst(value);

        rttr::instance obj = inst.get_type().get_raw_type().is_wrapper() ? inst.get_wrapped_instance() : inst;

        if(ImGui::BeginPopupContextItem())
        {
            if(ImGui::MenuItem("Copy"))
            {
                glfwSetClipboardString(NULL, Serializer::SerializeValue(value).c_str());
            }

            if(ImGui::MenuItem("Paste"))
            {
                Serializer::DeserializeValue(glfwGetClipboardString(NULL), value);
                prop.set_value(component, value);
            }
                
            ImGui::EndPopup();
        }

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
        else if(value.is_sequential_container())
        {
            auto View = value.create_sequential_view();
            int NumItems = View.get_size();

            bool bPropertyChanged = false;
            int indexToDelete = -1;
            int indexToInsert = -1;
            bool bAddToEnd = false;

            bool isFixedSize = !View.is_dynamic();

            ImGui::Text("%d Array elements", NumItems);
            
            if(!isFixedSize)
            {
                ImGui::SameLine();
                if(ImGui::Button("Clear"))
                {
                    View.clear();
                    bPropertyChanged = true;
                }
                
                ImGui::SameLine();
                if(ImGui::Button("Add"))
                {
                    bAddToEnd = true;
                }
            }
            
            if(bNameWidgetOpen)
            {
                ImGui::Indent(20.0f);
                
                for(int i = 0; i < View.get_size(); i++)
                {
                    std::string indexAsString = std::to_string(i);
                    std::string uniqueID = propName + "_" + indexAsString;
                    
                    ImGui::PushID(uniqueID.c_str());
                    
                    ImGui::TableNextColumn();
                    CreateLabelWidget(indexAsString.c_str(), "");

                    ImGui::TableNextColumn();
                    auto WrappedVar = View.get_value(i).extract_wrapped_value();
                    
                    if(ImGui::BeginPopupContextItem())
                    {
                        if(ImGui::MenuItem("Copy"))
                        {
                            glfwSetClipboardString(NULL, Serializer::SerializeValue(WrappedVar).c_str());
                        }

                        if(ImGui::MenuItem("Paste"))
                        {
                            Serializer::DeserializeValue(glfwGetClipboardString(NULL), WrappedVar);
                            bPropertyChanged = true;
                            View.set_value(i, WrappedVar);
                        }
                
                        ImGui::EndPopup();
                    }
                  
                    if(CreateValueWidget(WrappedVar, prop))
                    {                    
                        bPropertyChanged = true;
                        View.set_value(i, WrappedVar);
                    }

                    if(!isFixedSize)
                    {
                        ImGui::SameLine();
                        if(ImGui::Button("Del"))
                        {
                            indexToDelete = i;
                        }

                        ImGui::SameLine();
                        if(ImGui::Button("Insert"))
                        {
                            indexToInsert = i;
                        }
                    }

                    ImGui::PopID();
                }                
                ImGui::Unindent(20.0f);
                           
                ImGui::TreePop();
            }

            if(!isFixedSize)
            {
                if(indexToDelete >= 0)
                {
                    auto itr = View.begin();
                    for(int i = 0; i < indexToDelete; ++i)
                        ++itr;
                    View.erase(itr);
                    bPropertyChanged = true;
                }
                else if(indexToInsert >= 0)
                {
                    const rttr::type ArrayType = View.get_rank_type(1);
                    auto var = CreateDefaultVarFromType(ArrayType);
                    auto itr = View.begin();
                    for(int i = 0; i < indexToInsert; ++i)
                        ++itr;
                    View.insert(itr, var);
                    bPropertyChanged = true;
                }
                else if(bAddToEnd)
                {
                    const rttr::type ArrayType = View.get_rank_type(1);
                    auto var = CreateDefaultVarFromType(ArrayType);
                    View.insert(View.end(), var);
                    bPropertyChanged = true;
                }
            }

            if(bPropertyChanged)
                prop.set_value(component, value);
        }
        else if(value.is_associative_container())
        {
            auto View = value.create_associative_view();
            int NumItems = View.get_size();

            bool bPropertyChanged = false;
            rttr::variant KeyToDelete;
            bool bDeleteRequested = false;

            static std::map<std::string, bool> showAddRowMap;
            static std::map<std::string, rttr::variant> newKeyMap;
            static std::map<std::string, rttr::variant> newValueMap;
            
            std::string mapID = propName;
            bool& showAddRow = showAddRowMap[mapID];

            ImGui::Text("%d Map Elements", NumItems);
            ImGui::SameLine();
            if(ImGui::Button("Clear"))
            {
                View.clear();
                showAddRow = false;
                bPropertyChanged = true;
            }

            ImGui::SameLine();
            if(ImGui::Button("Add"))
            {
                if(!showAddRow)
                {
                    newKeyMap[mapID] = CreateDefaultVarFromType(View.get_key_type());
                    newValueMap[mapID] = CreateDefaultVarFromType(View.get_value_type());
                    showAddRow = true;
                    bForceExpand = true;
                }
            }

            if(bNameWidgetOpen)
            {
                ImGui::Indent(20.0f);

                int i = 0;
                std::vector<std::pair<rttr::variant, rttr::variant>> updatedEntries;
                
                for(auto& Item : View)
                {
                    std::string uniqueID = propName + "_map_" + std::to_string(i);
                    ImGui::PushID(uniqueID.c_str());

                    auto KeyVar = Item.first.extract_wrapped_value();
                    auto ValueVar = Item.second.extract_wrapped_value();

                    ImGui::TableNextColumn();
                    ImGui::PushID("key");
                    CreateValueWidget(KeyVar, prop, true);
                    ImGui::PopID();

                    ImGui::TableNextColumn();
                    ImGui::PushID("value");
                    if(CreateValueWidget(ValueVar, prop, false))
                    {
                        updatedEntries.push_back({KeyVar, ValueVar});
                    }
                    ImGui::PopID();

                    ImGui::SameLine();
                    if(ImGui::Button("Del"))
                    {
                        bDeleteRequested = true;
                        KeyToDelete = Item.first.extract_wrapped_value();
                    }

                    ImGui::PopID();

                    i++;
                }

                if(showAddRow)
                {
                    ImGui::PushID("new_entry");
                    
                    rttr::variant& newKey = newKeyMap[mapID];
                    rttr::variant& newValue = newValueMap[mapID];
                    
                    bool keyExists = View.find(newKey) != View.end();
                    bool keyEmpty = newKey.to_string().empty();
                    bool keyInvalid = keyExists || keyEmpty;
                    
                    ImGui::TableNextColumn();
                    ImGui::PushID("new_key");
                    if(keyInvalid)
                    {
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.8f, 0.3f, 0.3f, 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.9f, 0.4f, 0.4f, 0.7f));
                        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(1.0f, 0.5f, 0.5f, 0.9f));
                    }
                    CreateValueWidget(newKey, prop, false);
                    if(keyInvalid)
                    {
                        ImGui::PopStyleColor(3);
                        if(ImGui::IsItemHovered())
                        {
                            if(keyEmpty)
                                ImGui::SetTooltip("Key cannot be empty");
                            else
                                ImGui::SetTooltip("Key already exists in map");
                        }
                    }
                    ImGui::PopID();

                    ImGui::TableNextColumn();
                    ImGui::PushID("new_value");
                    CreateValueWidget(newValue, prop, false);
                    ImGui::PopID();

                    ImGui::SameLine();
                    if(keyInvalid)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
                    }
                    else
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
                    }
                    
                    if(ImGui::Button("OK") && !keyInvalid)
                    {
                        View.insert(newKey, newValue);
                        showAddRow = false;
                        bPropertyChanged = true;
                    }
                    ImGui::PopStyleColor(3);

                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
                    if(ImGui::Button("Cancel"))
                    {
                        showAddRow = false;
                    }
                    ImGui::PopStyleColor(3);

                    ImGui::PopID();
                }

                for(const auto& entry : updatedEntries)
                {
                    View.erase(entry.first);
                    View.insert(entry.first, entry.second);
                    bPropertyChanged = true;
                }

                ImGui::Unindent(20.0f);
                ImGui::TreePop();               
            }
            else if(showAddRow)
            {
                showAddRow = false;
            }

            if(bDeleteRequested)
            {
                View.erase(KeyToDelete);
                bPropertyChanged = true;
            }

            if(bPropertyChanged)
                prop.set_value(component, value);
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

    // CreateComponentSection template function moved to ComponentSectionHelper.h
 
    // Initialize the component registry using auto-generated code
    static void InitializeComponentRegistryInternal()
    {
        if (IsComponentRegistryInitialized())
            return;
            
        // Use the auto-generated component registry
        InitializeGeneratedComponentRegistry();
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        // Initialize the component registry if not already done
        InitializeComponentRegistryInternal();
        
        const auto& componentRegistry = GetComponentRegistry();
        
        // Find and draw TagComponent first (non-removable)
        for (const auto& componentInfo : componentRegistry)
        {
            if (componentInfo.type == rttr::type::get<TagComponent>())
            {
                if (componentInfo.hasComponent(entity))
                {
                    componentInfo.drawSection(entity, componentInfo.isRemovable);
                }
                break;
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if(ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");

        if(ImGui::BeginPopup("AddComponent"))
        {
            // Automatically create menu items for all addable component types
            for (const auto& componentInfo : componentRegistry)
            {
                // Skip TagComponent and TransformComponent as they're always present
                if (componentInfo.type == rttr::type::get<TagComponent>() || 
                    componentInfo.type == rttr::type::get<TransformComponent>())
                    continue;
                
                // Check if entity already has this component
                if (!componentInfo.hasComponent(entity))
                {
                    if (ImGui::MenuItem(componentInfo.displayName.c_str()))
                    {
                        componentInfo.addComponent(m_SelectedEntity);
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
                
            ImGui::EndPopup();
        }
        ImGui::PopItemWidth();

        // Draw all component sections automatically
        for (const auto& componentInfo : componentRegistry)
        {
            // Skip TagComponent as it's already drawn above
            if (componentInfo.type == rttr::type::get<TagComponent>())
                continue;
                
            if (componentInfo.hasComponent(entity))
            {
                componentInfo.drawSection(entity, componentInfo.isRemovable);
            }
        }
    }

    // Explicit template instantiations - auto-generated by HeaderTool
}
