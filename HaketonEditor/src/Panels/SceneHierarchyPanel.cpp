#include "SceneHierarchyPanel.h"

#include <string>

#include "Haketon/Scene/Scene.h"
#include "Haketon/Scene/Components.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_internal.h"


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

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, bool isRemovable = true)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

        if(entity.HasComponent<T>())
        {
            auto& component = entity.GetComponent<T>();
            
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
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
                uiFunction(component);
                ImGui::TreePop();
            }

            if(removeComponent && isRemovable)
                entity.RemoveComponent<T>();
        }
    }

    static void CreatePropertySection(rttr::property& prop, rttr::instance& component)
    {
        std::string propertyName = prop.get_name().to_string();
        auto propValue = prop.get_value(component);
        auto label = "##" + propertyName;

        rttr::variant degMeta = prop.get_metadata("Degrees");
        bool convertToDegrees = degMeta.get_value<bool>();

        // TODO: Add copy to clipboard functionality!
        
        ImGui::Text(propertyName.c_str());
        ImGui::NextColumn();
             
        if(propValue.is_type<int>())
        {
            int value = propValue.get_value<int>();
            if(ImGui::DragInt(label.c_str(), &value))
                prop.set_value(component, value);
        }
        else if(propValue.is_type<float>())
        {           
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
        /*else
        {
            ImGui::NextColumn();
            auto type = propValue.get_type();
            for(auto childProp : type.get_properties())
            {
                void* obj = propValue.get_value<void*>();
                rttr::instance instance(obj);
                CreatePropertySection(childProp, instance);
                ImGui::NextColumn();
            }
        }*/

        ImGui::NextColumn();
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

                ImGui::Columns(2, "myColumns");
                if(!ColumnsInitialized)
                {
                    ImGui::SetColumnWidth(0, 100.0f);
                    ColumnsInitialized = true;
                }
                
                for(auto prop : t.get_properties())
                    CreatePropertySection(prop, compInstance);

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
                
            ImGui::EndPopup();
        }
        ImGui::PopItemWidth();

        CreateComponentSection<TransformComponent>(entity, false);

        CreateComponentSection<CameraComponent>(entity, false, [](auto& component)
        {
            auto& camera = component.Camera;
            rttr::type t = rttr::type::get(camera);
            rttr::instance inst(camera);
            for(auto cameraProp : t.get_properties())
            {
                CreatePropertySection(cameraProp, inst);
            }
        });
        /*DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
        {
            auto& camera = component.Camera;

            ImGui::Checkbox("Primary", &component.Primary);

            const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
            const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
            if(ImGui::BeginCombo("Projection", currentProjectionTypeString))
            {
                for(int i = 0; i < 2; i++)
                {
                    bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                    if(ImGui::Selectable(projectionTypeStrings[i], isSelected))
                    {
                        currentProjectionTypeString = projectionTypeStrings[i];
                        camera.SetProjectionType((SceneCamera::ProjectionType)i);
                    }

                    if(isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                
                ImGui::EndCombo();
            }

            if(camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
            {
                float fov = glm::degrees(camera.GetPerspectiveVerticalFOV());
                if(ImGui::DragFloat("FOV", &fov))
                    camera.SetPerspectiveVerticalFOV(glm::radians(fov));

                float perspectiveNear = camera.GetPerspectiveNearClip();
                if(ImGui::DragFloat("Near Clip", &perspectiveNear))
                    camera.SetPerspectiveNearClip(perspectiveNear);

                float perspectiveFar = camera.GetPerspectiveFarClip();
                if(ImGui::DragFloat("Far Clip", &perspectiveFar))
                    camera.SetPerspectiveFarClip(perspectiveFar);   
            }
            else if(camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
            {
                float orthoSize = camera.GetOrthographicSize();
                if(ImGui::DragFloat("Size", &orthoSize))
                    camera.SetOrthographicSize(orthoSize);

                float orthoNear = camera.GetOrthographicNearClip();
                if(ImGui::DragFloat("Near Clip", &orthoNear))
                    camera.SetOrthographicNearClip(orthoNear);

                float orthoFar = camera.GetOrthographicFarClip();
                if(ImGui::DragFloat("Far Clip", &orthoFar))
                    camera.SetOrthographicFarClip(orthoFar);

                ImGui::Checkbox("Fixed Aspect Ration", &component.FixedAspectRatio);
            }
        });*/

        CreateComponentSection<SpriteRendererComponent>(entity, true); 
    }
}
