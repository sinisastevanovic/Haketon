#pragma once
#include <filesystem>

#include "Haketon/Renderer/Texture.h"

namespace Haketon
{
    class AssetBrowser
    {
    public:
        AssetBrowser();

        void OnImGuiRender();

        void SetAssetDirectory(const std::filesystem::path& path);

    private:

        void RenderDeleteConfirmationDialog();
        void RenderNewDirectoryDialog();

        std::filesystem::path m_AssetDirectoryRoot;
        std::filesystem::path m_CurrentDirectory;

        Ref<Texture2D> m_DirectoryIcon;
        Ref<Texture2D> m_GenericFileIcon;

        std::unordered_set<UUID> m_SelectedAssets;
        std::unordered_set<std::filesystem::path> m_SelectedDirectories;

        bool m_DeleteConfirmationVisible = false;
        bool m_NewDirectoryVisible = false;
        std::string m_NewDirectoryName;
    };

}
