#include "hkpch.h"
#include "Asset.h"

namespace Haketon
{
    Asset::Asset()
        : m_Handle(AssetHandle()), m_IsTransient(true)
    {
    }

    Asset::Asset(const AssetHandle& handle, const std::string& path, const std::string& name)
        : m_Handle(handle), m_Path(path), m_Name(name)
    {
    }
}
