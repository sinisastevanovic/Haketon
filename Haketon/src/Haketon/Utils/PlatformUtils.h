#pragma once

#include <string>

namespace Haketon
{
    class FileDialogs
    {
    public:

        // Returns empty string if cancelled
        static HK_API std::string OpenFile(const char* Filter);

        static HK_API std::string OpenAsset();

        // Returns empty string if cancelled
        static HK_API std::string SaveFile(const char* Filter);

    };
}