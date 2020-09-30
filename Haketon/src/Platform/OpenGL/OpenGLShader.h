#pragma once

#include "Haketon/Renderer/Shader.h"

namespace Haketon {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		void Bind() const override;
		void Unbind() const override;

		void SetMat4(const ::std::string& name, const ::glm::mat4& value) override;
	private:
		uint32_t m_RendererID;
	};
}