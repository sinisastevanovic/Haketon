#include "AssetBrowser.h"
#include "Haketon/Core/Asset/AssetManager.h"

#include <algorithm>
#include <imgui.h>

namespace Haketon
{
    AssetBrowser::AssetBrowser()
    {
        m_DirectoryIcon = Texture2D::Create("resources/icons/Folder.png");
        m_GenericFileIcon = Texture2D::Create("resources/icons/File.png");
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
        ImGui::Separator();

        // Main content area
        float padding = 16.0f;
        float thumbnailSize = 96.0f;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
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
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::ImageButton("1", (ImTextureID)(uint64_t)m_DirectoryIcon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                nextPath /= path.filename();
            }

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

            // TODO: Use correct icon based on asset type
            Ref<Texture2D> icon = m_GenericFileIcon;
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::ImageButton("1", (ImTextureID)(uint64_t)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
            ImGui::PopStyleColor();

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
        ImGui::End();
    }

    void AssetBrowser::SetAssetDirectory(const std::filesystem::path& path)
    {
        m_AssetDirectoryRoot = path;
        m_CurrentDirectory = m_AssetDirectoryRoot;
    }
}
