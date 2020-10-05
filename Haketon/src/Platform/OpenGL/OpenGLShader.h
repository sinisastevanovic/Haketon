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

		void SetInt(const std::string& name, int value) override;

		void SetFloat1(const std::string& name, float value) override;
		void SetFloat2(const std::string& name, const glm::vec2& value) override;
		void SetFloat3(const ::std::string& name, const ::glm::vec3& value) override;
		void SetFloat4(const std::string& name, const ::glm::vec4& value) override;

		void SetMat3(const std::string& name, const glm::mat3& value) override;
		void SetMat4(const std::string& name, const ::glm::mat4& value) override;
	private:
		uint32_t m_RendererID;
	};
}