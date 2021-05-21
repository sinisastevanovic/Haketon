#pragma once

namespace Haketon
{
    struct FWindowsPlatformMisc
    {
    public:
        static void CreateGuid(struct FGuid& Result);
    
    };

#ifdef HK_PLATFORM_WINDOWS
    typedef FWindowsPlatformMisc FPlatformMisc;
#endif
}
