#pragma once
#include "Haketon/Events/Event.h"
#include "Project/Project.h"

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
        SceneOpenEvent() = default;

        EVENT_CLASS_TYPE(SceneOpenEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

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

    class ProjectNewEvent : public Event
    {
    public:
        ProjectNewEvent() = default;

        EVENT_CLASS_TYPE(ProjectNewEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class ProjectOpenEvent : public Event
    {
    public:
        ProjectOpenEvent() = default;

        EVENT_CLASS_TYPE(ProjectOpenEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class CurrentProjectChangedEvent : public Event
    {
    public:
        CurrentProjectChangedEvent(Project* project) : m_Project(project) {}

        Project* GetProject() const { return m_Project; }

        EVENT_CLASS_TYPE(CurrentProjectChangedEvent)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    private:
        Project* m_Project = nullptr;
    };
}
