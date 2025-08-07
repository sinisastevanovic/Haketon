#include "hkpch.h"
#include "ScriptableEntity.h"

namespace Haketon
{
    void ScriptableEntity::SetData(std::vector<ScriptData> data)
    {
        auto thisType = rttr::type::get(*this);
        for (auto i : data)
        {
            thisType.set_property_value(i.Name, rttr::instance(this), i.Value);
        }
    }
}
