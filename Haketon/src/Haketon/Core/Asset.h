#pragma once

#include "Haketon/Core/Core.h"
#include "IReflectable.h"
#include "Misc/UUID.h"

namespace Haketon
{
    CLASS(abstract)
    class HK_API Asset : public IReflectable
    {
    public:
        Asset();
        explicit Asset(const UUID& uuid);
        virtual ~Asset() = default;

        const UUID& GetUUID() const { return m_Uuid; }
        const std::string& GetName() const { return m_Name; }
        const std::string& GetPath() const { return m_Path; }

        void SetName(const std::string& name) { m_Name = name; SetDirty(true); }
        void SetPath(const std::string& path) { m_Path = path; SetDirty(true); }

        bool IsLoaded() const { return m_IsLoaded; }
        bool IsDirty() const { return m_IsDirty; }
        void SetDirty(bool dirty) { m_IsDirty = dirty; }

        virtual bool Load() = 0;
        virtual bool Unload() = 0;

    protected:
        UUID m_Uuid;
        std::string m_Name;
        std::string m_Path;
        bool m_IsLoaded = false;
        bool m_IsDirty = false;

        RTTR_ENABLE(IReflectable)
    };

}
