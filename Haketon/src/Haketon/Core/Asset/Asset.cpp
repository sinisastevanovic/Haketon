#include "hkpch.h"
#include "Asset.h"

namespace Haketon
{
    Asset::Asset()
    {
    }

    Asset::Asset(const AssetHandle& handle)
        : m_Handle(handle)
    {
    }
}
