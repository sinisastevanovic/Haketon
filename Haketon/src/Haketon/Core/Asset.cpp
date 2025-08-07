#include "hkpch.h"
#include "Asset.h"

namespace Haketon
{
    Asset::Asset()
    {
    }

    Asset::Asset(const UUID& uuid)
        : m_Uuid(uuid)
    {
    }
}
