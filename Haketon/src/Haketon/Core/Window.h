#pragma once

#include <sstream>

#include "Haketon/Core/Core.h"
#include "Haketon/Events/Event.h"

namespace Haketon
{
	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		bool Maximized = false;

		WindowProps(const std::string& title = "Haketon Engine", bool maximized = false,
			uint32_t width = 1600,
			uint32_t height = 900)
			: Title(title), Width(width), Height(height), Maximized(maximized) {}

	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		//Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}