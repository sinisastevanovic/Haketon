#pragma once

#include <functional>

#include "Haketon/Debug/Instrumentor.h"
#include "Haketon/Core/Core.h"
#include "Haketon/Core/TypeID.h"

namespace Haketon {

	// Events in Haketon are currently blocking, meaning when an event occurs it
	// immediately gets dispatched and must be dealt with right then and there.
	// For the future, a better strategy might be to buffer events in an event
	// bus and process them during the "event" part of the update stage.

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication	= BIT(0),
		EventCategoryInput			= BIT(1),
		EventCategoryKeyboard		= BIT(2),
		EventCategoryMouse			= BIT(3),
		EventCategoryMouseButton	= BIT(4)
	};

#define EVENT_CLASS_TYPE(type)                                                 \
	static Haketon::EventTypeID GetStaticType() {                              \
		return Haketon::TypeID<type>::Get();                                   \
	}                                                                          \
	virtual Haketon::EventTypeID GetEventType() const override {               \
		return GetStaticType();                                                \
	}                                                                          \
	virtual const char* GetName() const override { return #type; }
	
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class HK_API Event
	{
		friend class EventDispatcher;

	public:
		virtual ~Event() = default;
		
		virtual EventTypeID GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() &category;
		}

		static void Dispatch(Event& event);

		bool Handled = false;
	};

	class HK_API EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_Event(event) {}

		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled = func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}
