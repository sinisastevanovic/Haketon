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

	#ifdef HK_ENABLE_ASSERTS
		int versionMajor;
		int versionMinor;
		glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
		glGetIntegerv(GL_MINOR_VERSION, &versionMinor);

		HK_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Haketon requires OpenGL version 4.5 or higher!");
	#endif
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}