#pragma once

#include <string>

namespace Haketon
{
    class FileDialogs
    {
    public:

        // Returns empty string if cancelled
        static std::string OpenFile(const char* Filter);

        // Returns empty string if cancelled
        static std::string SaveFile(const char* Filter);

    };

    class PlatformMisc
    {
    public:
        static void CreateGuid(struct FGuid& Result);
    };
}