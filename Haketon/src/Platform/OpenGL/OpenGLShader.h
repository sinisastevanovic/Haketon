#pragma once

#include "Haketon/Renderer/Shader.h"

#include <glad/glad.h>

namespace Haketon {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filePath, const std::string& name = "");
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		void Bind() const override;
		void Unbind() const override;

		void SetInt(const std::string& name, int value) override;

		void SetFloat(const std::string& name, float value) override;
		void SetFloat2(const std::string& name, const glm::vec2& value) override;
		void SetFloat3(const ::std::string& name, const ::glm::vec3& value) override;
		void SetFloat4(const std::string& name, const ::glm::vec4& value) override;

		void SetMat3(const std::string& name, const glm::mat3& value) override;
		void SetMat4(const std::string& name, const ::glm::mat4& value) override;

		virtual const std::string& GetName() const override { return m_Name; }
	private:
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
	private:
		uint32_t m_RendererID;

		// TODO: Use glObjectLabel instead of string member. This allows you to see the name from external opengl debugging tools as well.
		std::string m_Name;
	};
}