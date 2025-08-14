#pragma once

#include "Haketon/Core/Core.h"
#include "Haketon/Core/IReflectable.h"
#include "AssetHandle.h"
#include "AssetTypes.h"

namespace Haketon
{
    CLASS(abstract)
    class HK_API Asset : public IReflectable
    {
    public:
        Asset();
        explicit Asset(const AssetHandle& handle);
        virtual ~Asset() = default;

        const AssetHandle& GetHandle() const { return m_Handle; }
        const std::string& GetName() const { return m_Name; }
        const std::string& GetPath() const { return m_Path; }

        void SetName(const std::string& name) { m_Name = name; SetDirty(true); }
        void SetPath(const std::string& path) { m_Path = path; SetDirty(true); }

        bool IsDirty() const { return m_IsDirty; }
        void SetDirty(bool dirty) { m_IsDirty = dirty; }

        virtual AssetType GetType() const = 0;

    protected:
        AssetHandle m_Handle = AssetHandle::Null();
        std::string m_Name;
        std::string m_Path;
        bool m_IsDirty = false;

        RTTR_ENABLE(IReflectable)

        friend class AssetManager;
    };

}
