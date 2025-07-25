#include "hkpch.h"
#include "WindowsWindow.h"

#include "Haketon/Events/ApplicationEvent.h"
#include "Haketon/Events/MouseEvent.h"
#include "Haketon/Events/KeyEvent.h"
#include "Haketon/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLContext.h"

namespace Haketon
{
	static uint8_t s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		HK_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}
	
	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		HK_PROFILE_FUNCTION();
		
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		HK_PROFILE_FUNCTION();

		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		HK_PROFILE_FUNCTION();
		
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;


		HK_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (s_GLFWWindowCount == 0)
		{
			HK_CORE_INFO("Initializing GLFW");
			int success = glfwInit();
			HK_CORE_ASSERT(success, "Could not initialize GLFW");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

	#if defined(HK_DEBUG)
		if(Renderer::GetAPI() == RendererAPI::API::OpenGL)
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	#endif

		if(props.Maximized)
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
		
		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		++s_GLFWWindowCount;

		if(props.Maximized)
		{
			int width, height;
			glfwGetWindowSize(m_Window, &width, &height);
			m_Data.Width = width;
			m_Data.Height = height;

			glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
		}

		m_Context = CreateScope<OpenGLContext>(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		/*----- Set GLFW callbacks -----*/
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);

		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(static_cast<KeyCode>(key));
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), 1);	//TODO: If we need the repeat amount, we'll have to extract it... didn't bother now
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			KeyTypedEvent event(static_cast<KeyCode>(keycode));
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(static_cast<MouseCode>(button));
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double  yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event((float)xPos, (float)yPos);
				data.EventCallback(event);
			});
	}

	void WindowsWindow::Shutdown()
	{
		HK_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);

		if(--s_GLFWWindowCount == 0)
		{
			HK_CORE_INFO("Terminating GLFW");
			glfwTerminate();
		}
	}

	void WindowsWindow::OnWindowResized(GLFWwindow* window, int width, int height)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.Width = width;
		data.Height = height;

		WindowResizeEvent event(width, height);
		data.EventCallback(event);

		HK_CORE_TRACE("{0}", width);
	}

	void WindowsWindow::OnUpdate()
	{
		HK_PROFILE_FUNCTION();

		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		HK_PROFILE_FUNCTION();

		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}



}