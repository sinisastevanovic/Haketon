#include "hkpch.h"
#include "ComponentRegistry.h"

namespace Haketon
{
    ComponentRegistry& ComponentRegistry::instance()
    {
        static ComponentRegistry instance;
        return instance;
    }

}
