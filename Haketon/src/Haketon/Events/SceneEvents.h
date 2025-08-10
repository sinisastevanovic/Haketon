#pragma once

#include "Haketon/Events/Event.h"

namespace Haketon
{
    class ScenePlayEvent : public Event
    {
    public:
        ScenePlayEvent() = default;

        EVENT_CLASS_TYPE(ScenePlayEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class ScenePauseEvent : public Event
    {
    public:
        ScenePauseEvent(bool paused) : m_Paused(paused) {}

        bool Paused() const { return m_Paused; }

        EVENT_CLASS_TYPE(ScenePauseEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        bool m_Paused = false;
    };

    class SceneStopEvent : public Event
    {
    public:
        SceneStopEvent() = default;

        EVENT_CLASS_TYPE(SceneStopEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };
}
