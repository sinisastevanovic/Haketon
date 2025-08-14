#pragma once
#include "Haketon/Events/Event.h"
#include "Haketon/Core/Asset/AssetHandle.h"

namespace Haketon
{
    class AssetOpenEvent : public Event
    {
    public:
        AssetOpenEvent(AssetHandle handle) : m_Handle(handle) {}

        AssetHandle GetHandle() const { return m_Handle; }

        EVENT_CLASS_TYPE(AssetOpenEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        AssetHandle m_Handle;
    };
}