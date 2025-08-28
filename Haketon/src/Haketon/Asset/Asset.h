#pragma once

#include "Haketon/Core/Core.h"
#include "Haketon/Core/IReflectable.h"
#include "AssetHandle.h"
#include "AssetTypes.h"
#include "Haketon/Core/PathUtils.h"

namespace Haketon
{
    CLASS(abstract)
    class HK_API Asset : public IReflectable
    {
    public:
        virtual ~Asset() = default;

        const AssetHandle& GetHandle() const { return m_Handle; }
        const std::string& GetName() const { return m_Name; }
        const std::filesystem::path& GetPath() const { return m_Path; }
        std::filesystem::path GetFullPath() const { return PathUtils::GetGameAssetsPath() / m_Path; }

        void SetName(const std::string& name) { m_Name = name; SetDirty(true); }
        void SetPath(const std::string& path) { m_Path = path; SetDirty(true); }

        bool IsDirty() const { return m_IsDirty; }
        void SetDirty(bool dirty) { m_IsDirty = dirty; }

        virtual AssetType GetType() const = 0;

        bool operator==(const Asset& other) const { return m_Handle == other.m_Handle; }
        bool operator!=(const Asset& other) const { return m_Handle != other.m_Handle; }

    protected:
        AssetHandle m_Handle = AssetHandle::Null();
        std::filesystem::path m_Path;
        std::string m_Name;
        bool m_IsDirty = false;

        RTTR_ENABLE(IReflectable)

        friend class AssetManager;
    };

}
