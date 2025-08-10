#pragma once

#include "Core.h"

namespace Haketon
{
    using EventTypeID = uint32_t;

    HK_API EventTypeID GetNextEventTypeID();

    template <typename T>
    class TypeID
    {
    public:
        static EventTypeID Get()
        {
            static EventTypeID s_ID = GetNextEventTypeID();
            return s_ID;
        }
    };
}