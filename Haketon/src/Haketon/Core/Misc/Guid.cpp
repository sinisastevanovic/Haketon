#include "hkpch.h"
#include "Guid.h"

#include "Haketon/Utils/PlatformUtils.h"

namespace Haketon
{
    std::string FGuid::ToString() const
    {
        return fmt::format("{0:08X}{1:08X}{2:08X}{3:08X}", A, B, C, D);
    }

    FGuid FGuid::NewGuid()
    {
        FGuid Result(0, 0, 0, 0);
        PlatformMisc::CreateGuid(Result);
       
        return Result;
    }

    bool FGuid::Parse(const std::string& GuidString, FGuid& OutGuid)
    {
        HK_CORE_ASSERT(GuidString.length() == 32, "Trying to Parse Guid with invalid string!");

        for(int32_t i = 0; i < GuidString.length(); ++i)
        {
            if(!isxdigit(GuidString[i]))
                return false;
        }
        
        OutGuid = FGuid(
            std::stoul(GuidString.substr(0, 8), nullptr, 16),
            std::stoul(GuidString.substr(8, 8), nullptr, 16),
            std::stoul(GuidString.substr(16, 8), nullptr, 16),
            std::stoul(GuidString.substr(24, 8), nullptr, 16)
        );

        return true;
    }
}
