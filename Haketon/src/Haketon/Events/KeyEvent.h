#pragma once

#include "Haketon/Core/Core.h"
#include "Event.h"
#include "Haketon/Core/KeyCodes.h"

namespace Haketon {

	class HK_API KeyEvent : public Event
	{
	public:
		
		KeyCode GetKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:
		KeyEvent(const KeyCode keycode)
			: m_KeyCode(keycode) {}

		KeyCode m_KeyCode;
	};

	class HK_API KeyPressedEvent : public KeyEvent
	{
	public:

		KeyPressedEvent(const KeyCode keycode, const uint16_t repeatCount)
			: KeyEvent(keycode), m_RepeatCount(repeatCount) {}

		uint16_t GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressedEvent)

	private:
		uint16_t m_RepeatCount;
	};

	class HK_API KeyReleasedEvent : public KeyEvent
	{
	public:

		KeyReleasedEvent(const KeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleasedEvent)
	};

	class HK_API KeyTypedEvent : public KeyEvent
	{
	public:

		KeyTypedEvent(const KeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode ;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTypedEvent)
	};
}