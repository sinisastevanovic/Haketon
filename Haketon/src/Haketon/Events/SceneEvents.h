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
        ScenePauseEvent() = default;

        EVENT_CLASS_TYPE(ScenePauseEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class SceneStopEvent : public Event
    {
    public:
        SceneStopEvent() = default;

        EVENT_CLASS_TYPE(SceneStopEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };
}
