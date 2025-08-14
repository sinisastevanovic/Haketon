#include "AssetBrowser.h"
#include "Haketon/Core/Asset/AssetManager.h"

#include <algorithm>
#include <imgui.h>

#include "EditorLayer.h"
#include "Events/EditorAssetEvents.h"
#include "Haketon/Utils/PlatformUtils.h"

namespace Haketon
{
    struct AssetBrowserDragDropPayload
    {
        std::vector<UUID> AssetHandles;
        std::vector<std::filesystem::path> DirectoryPaths;
    };

    
    AssetBrowser::AssetBrowser()
    {
        m_DirectoryIcon = Texture2D::Create("resources/icons/Folder.png", false);
        m_GenericFileIcon = Texture2D::Create("resources/icons/File.png", false);
        //m_GenericFileIcon = Texture2D::Create("resources/icons/File.png");
    }

    void AssetBrowser::OnImGuiRender()
    {
        ImGui::Begin("Asset Browser");

        if (m_AssetDirectoryRoot.empty())
        {
            ImGui::Text("Empty");
            ImGui::End();
            return;
        }

        ImGui::BeginDisabled(m_CurrentDirectory == m_AssetDirectoryRoot);
        if (ImGui::Button("<- Back"))
        {
            m_CurrentDirectory = m_CurrentDirectory.parent_path();
        }
        ImGui::EndDisabled();
        
        ImGui::SameLine();
        ImGui::Text("%s", m_CurrentDirectory.string().c_str());
        if (AssetManager::FoundUnimportedAssets())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.0f, 0.0f, 1.0f });
            ImGui::Text("Unimported Assets found in asset directory");
            ImGui::PopStyleColor();
            if (ImGui::Button("Import"))
            {
                AssetManager::ImportUnimportedAssets();
            }
        }
        ImGui::Separator();

        if (ImGui::BeginPopupContextWindow("AssetBrowser_ContextMenu_Background"))
        {
            if (ImGui::MenuItem("Create Folder"))
            {
                m_NewDirectoryVisible = true;
            }
            if (ImGui::MenuItem("Import New Asset..."))
            {
                std::string assetPath = FileDialogs::OpenAsset();
                AssetManager::ImportAsset(assetPath);
            }
            ImGui::EndPopup();
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
        {
            m_SelectedAssets.clear();
            m_SelectedDirectories.clear();
        }

        // Main content area
        float padding = 16.0f;
        float thumbnailSize = 96.0f;
        float cellSizeX = thumbnailSize + padding;
        float textSize = 8.0f;
        float cellSizeY = thumbnailSize + padding + textSize;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSizeX);
        columnCount = std::max(columnCount, 1);

        ImGui::Columns(columnCount, 0, false);

        std::filesystem::path nextPath = m_CurrentDirectory;
        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            if (!directoryEntry.is_directory())
                continue;
            
            const auto& path = directoryEntry.path();
            std::string filenameString = path.filename().string();

            ImGui::PushID(filenameString.c_str());

            bool isSelected = m_SelectedDirectories.contains(path);

            if (ImGui::Selectable("##directory_selectable", isSelected, ImGuiSelectableFlags_AllowDoubleClick, { thumbnailSize, cellSizeY }))
            {
                if (!ImGui::GetIO().KeyCtrl)
                {
                    m_SelectedAssets.clear();
                    m_SelectedDirectories.clear();
                }

                if (isSelected)
                    m_SelectedDirectories.erase(path);
                else
                    m_SelectedDirectories.insert(path);

                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    nextPath /= path.filename();
                }
            }

            if (ImGui::BeginPopupContextItem("AssetBrowser_ContextMenu_Item"))
            {
                if (!isSelected)
                {
                    m_SelectedAssets.clear();
                    m_SelectedDirectories.clear();
                    m_SelectedDirectories.insert(path);
                }

                if (ImGui::MenuItem("Delete"))
                {
                    m_DeleteConfirmationVisible = true;
                }
                ImGui::EndPopup();
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_PAYLOAD"))
                {
                    auto* data = (AssetBrowserDragDropPayload*)payload->Data;
                    for (const auto& handle : data->AssetHandles)
                    {
                        AssetManager::MoveAsset(handle, path);
                    }
                    // TODO: Implement moving directories
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SetCursorScreenPos({ ImGui::GetItemRectMin().x + padding / 2, ImGui::GetItemRectMin().y + padding / 2 });
            ImGui::Image((ImTextureID)(uint64_t)m_DirectoryIcon->GetRendererID(), { thumbnailSize - padding, thumbnailSize - padding }, { 0, 1 }, { 1, 0 });

            ImGui::TextWrapped("%s", filenameString.c_str());

            ImGui::NextColumn();
            ImGui::PopID();
        }

        auto assetHandles = AssetManager::GetAssetsInDirectory(m_CurrentDirectory);
        for (const auto& handle : assetHandles)
        {
            const AssetMetadata* metadata = AssetManager::GetMetadata(handle);
            if (!metadata)
                continue;

            std::string filenameString = metadata->SourceFilePath.filename().string();
            ImGui::PushID(filenameString.c_str());

            bool isSelected = m_SelectedAssets.contains(handle);

            if (ImGui::Selectable("##asset_selectable", isSelected, ImGuiSelectableFlags_AllowDoubleClick, { thumbnailSize, thumbnailSize }))
            {
                if (!ImGui::GetIO().KeyCtrl)
                {
                    m_SelectedAssets.clear();
                    m_SelectedDirectories.clear();
                }

                if (isSelected)
                    m_SelectedAssets.erase(handle);
                else
                    m_SelectedAssets.insert(handle);

                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    AssetOpenEvent event(handle);
                    Event::Dispatch(event);
                }
            }

            if (ImGui::BeginPopupContextItem("AssetBrowser_ContextMenu_Item"))
            {
                if (!isSelected)
                {
                    m_SelectedAssets.clear();
                    m_SelectedDirectories.clear();
                    m_SelectedAssets.insert(handle);
                }

                if (ImGui::MenuItem("Delete"))
                {
                    m_DeleteConfirmationVisible = true;
                }
                ImGui::EndPopup();
            }

            if (ImGui::BeginDragDropSource())
            {
                static AssetBrowserDragDropPayload payloadData;
                payloadData.AssetHandles.assign(m_SelectedAssets.begin(), m_SelectedAssets.end());
                payloadData.DirectoryPaths.assign(m_SelectedDirectories.begin(), m_SelectedDirectories.end());
                ImGui::SetDragDropPayload("ASSET_BROWSER_PAYLOAD", &payloadData, sizeof(AssetBrowserDragDropPayload));
                ImGui::Text("Dragging %zu items", payloadData.AssetHandles.size() + payloadData.DirectoryPaths.size());
                ImGui::EndDragDropSource();
            }

            ImGui::SetCursorScreenPos({ ImGui::GetItemRectMin().x + padding / 2, ImGui::GetItemRectMin().y + padding / 2 });

            // TODO: Use correct icon based on asset type
            Ref<Texture2D> icon;
            switch (metadata->Type)
            {
                case AssetType::Texture:
                case AssetType::Mesh:
                case AssetType::Material:
                case AssetType::Scene:
                case AssetType::None:
                default: icon = m_GenericFileIcon; break;
            }
            
            ImGui::Image((ImTextureID)(uint64_t)icon->GetRendererID(), { thumbnailSize - padding, thumbnailSize - padding }, { 0, 1 }, { 1, 0 });

            ImGui::TextWrapped("%s", filenameString.c_str());
            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::Columns(1);

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("IM_GUI_PAYLOAD_TYPE_FILE_DRAG_DROP"))
            {
                HK_CORE_INFO("Files dropped into the Asset Browser!");
            }
            ImGui::EndDragDropTarget();
        }

        m_CurrentDirectory = nextPath;

        RenderDeleteConfirmationDialog();
        RenderNewDirectoryDialog();
        
        ImGui::End();
    }

    void AssetBrowser::SetAssetDirectory(const std::filesystem::path& path)
    {
        m_AssetDirectoryRoot = path;
        m_CurrentDirectory = m_AssetDirectoryRoot;
    }

    void AssetBrowser::RenderDeleteConfirmationDialog()
    {
        if (m_DeleteConfirmationVisible)
        {
            ImGui::OpenPopup("Delete Confirmation");
            m_DeleteConfirmationVisible = false;
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Delete Confirmation", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            size_t totalItems = m_SelectedAssets.size() + m_SelectedDirectories.size();
            ImGui::Text("Are you sure you want to delete %zu selected item(s)?", totalItems);
            ImGui::Text("This action cannot be undone."); // TODO: Make it undoable
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                for (const auto& assetHandle : m_SelectedAssets)
                {
                    AssetManager::DeleteAsset(assetHandle);
                }
                for (const auto& dirPath : m_SelectedDirectories)
                {
                    AssetManager::DeleteDirectory(dirPath);
                }
                m_SelectedDirectories.clear();
                m_SelectedAssets.clear();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void AssetBrowser::RenderNewDirectoryDialog()
    {
        if (m_NewDirectoryVisible)
        {
            ImGui::OpenPopup("New Folder");
            m_NewDirectoryVisible = false;
            m_NewDirectoryName = "New Folder";
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("New Folder", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), m_NewDirectoryName.c_str());           
            if(ImGui::InputText("Name", buffer, sizeof(buffer)))
            {
                m_NewDirectoryName = std::string(buffer);
            }

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                AssetManager::CreateDir(m_CurrentDirectory / m_NewDirectoryName);
                ImGui::CloseCurrentPopup();
            }

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
}
