#include "hkpch.h"
#include "TypeID.h"

namespace Haketon
{
    static EventTypeID s_EventTypeCounter = 0;

    HK_API EventTypeID GetNextEventTypeID()
    {
        return s_EventTypeCounter++;
    }
}