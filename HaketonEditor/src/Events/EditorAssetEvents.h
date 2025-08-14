#pragma once
#include "Haketon/Core/Misc/UUID.h"
#include "Haketon/Events/Event.h"

namespace Haketon
{
    class AssetOpenEvent : public Event
    {
    public:
        AssetOpenEvent(UUID handle) : m_Handle(handle) {}

        UUID GetHandle() const { return m_Handle; }

        EVENT_CLASS_TYPE(AssetOpenEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        UUID m_Handle;
    };
}