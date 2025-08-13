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

        std::filesystem::path m_AssetDirectoryRoot;
        std::filesystem::path m_CurrentDirectory;

        Ref<Texture2D> m_DirectoryIcon;
        Ref<Texture2D> m_GenericFileIcon;
    };

}
