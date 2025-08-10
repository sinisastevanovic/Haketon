#pragma once
#include "Haketon/Events/Event.h"

namespace Haketon
{
    class ActiveSceneChangedEvent : public Event
    {
    public:
        ActiveSceneChangedEvent() = default;

        EVENT_CLASS_TYPE(ActiveSceneChangedEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class SceneOpenEvent : public Event
    {
    public:
        SceneOpenEvent(const std::string& path) : m_Path(path) {}

        const std::string& GetPath() const { return m_Path; }

        EVENT_CLASS_TYPE(SceneOpenEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        std::string m_Path;
    };

    class SceneNewEvent : public Event
    {
    public:
        SceneNewEvent() = default;

        EVENT_CLASS_TYPE(SceneNewEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class SceneSaveEvent : public Event
    {
    public:
        SceneSaveEvent() = default;

        EVENT_CLASS_TYPE(SceneSaveEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class SceneSaveAsEvent : public Event
    {
    public:
        SceneSaveAsEvent() = default;

        EVENT_CLASS_TYPE(SceneSaveAsEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };
}