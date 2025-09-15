#pragma once

#include "ISerializer.h"
#include <nlohmann/json.hpp>

namespace Haketon
{
    class HK_API JsonSerializer : public ISerializerNew
    {
    public:
        JsonSerializer();
        ~JsonSerializer();

        std::string Serialize(const rttr::variant& object) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;
    };
}
