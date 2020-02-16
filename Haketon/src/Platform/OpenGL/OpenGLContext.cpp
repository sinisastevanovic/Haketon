#include "hkpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Haketon {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		HK_CORE_ASSERT(windowHandle, "Windowhandle is null!")
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		HK_CORE_ASSERT(status, "Failed to initialize Glad!");

		HK_CORE_INFO("OpenGL Info");
		HK_CORE_INFO("   Vendor: {0}", glGetString(GL_VENDOR));
		HK_CORE_INFO("   Renderer: {0}", glGetString(GL_RENDERER));
		HK_CORE_INFO("   Version: {0}", glGetString(GL_VERSION));
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}