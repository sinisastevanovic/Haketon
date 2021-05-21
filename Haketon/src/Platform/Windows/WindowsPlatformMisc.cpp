#include "hkpch.h"
#include "WindowsPlatformMisc.h"

#include "combaseapi.h"

namespace Haketon
{
    void FWindowsPlatformMisc::CreateGuid(FGuid& Result)
    {
        CoCreateGuid((GUID*)&Result);
    }
}

