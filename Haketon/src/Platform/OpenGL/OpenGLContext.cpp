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
		HK_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		HK_CORE_ASSERT(status, "Failed to initialize Glad!");

		HK_CORE_INFO("OpenGL Info");
		HK_CORE_INFO("   Vendor: {0}", fmt::ptr(glGetString(GL_VENDOR)));
		HK_CORE_INFO("   Renderer: {0}", fmt::ptr(glGetString(GL_RENDERER)));
		HK_CORE_INFO("   Version: {0}", fmt::ptr(glGetString(GL_VERSION)));

		HK_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "Haketon requires OpenGL version 4.5 or higher!");
	}

	void OpenGLContext::SwapBuffers()
	{
		HK_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}
}