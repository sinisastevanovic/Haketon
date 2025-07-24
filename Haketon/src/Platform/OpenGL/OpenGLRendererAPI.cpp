#include "hkpch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

#ifdef HK_DEBUG
	#define HK_OPENGL_MESSAGES 1
#else
	#define HK_OPENGL_MESSAGES 0
#endif

namespace Haketon {

	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		switch(severity)
		{
			case GL_DEBUG_SEVERITY_HIGH: 			HK_CORE_CRIT(message); return;
			case GL_DEBUG_SEVERITY_MEDIUM:			HK_CORE_ERROR(message); return;
			case GL_DEBUG_SEVERITY_LOW:				HK_CORE_WARN(message); return;
			case GL_DEBUG_SEVERITY_NOTIFICATION: 	HK_CORE_TRACE(message); return;
		}

		HK_CORE_ASSERT(false, "Unknown severity level!");
	}
	
	void OpenGLRendererAPI::Init()
	{
	#if HK_OPENGL_MESSAGES
		#ifdef HK_DEBUG
				glEnable(GL_DEBUG_OUTPUT);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
				glDebugMessageCallback(OpenGLMessageCallback, nullptr);

				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
		#endif
	#endif	
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const FColor& color)
	{
		glClearColor(color.r(), color.g(), color.b(), color.a());
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		//glBindTexture(GL_TEXTURE_2D, 0); TODO: DO WE NEED THIS??
	}
}