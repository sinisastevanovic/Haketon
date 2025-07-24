#pragma once

#include "Haketon/Renderer/Shader.h"

#include <glad/glad.h>

namespace Haketon {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filePath);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		void Bind() const override;
		void Unbind() const override;

		void SetInt(const std::string& name, int value) override;
		void SetIntArray(const ::std::string& name, int* values, uint32_t count) override;
		
		void SetFloat(const std::string& name, float value) override;
		void SetFloat2(const std::string& name, const FVec2& value) override;
		void SetFloat3(const ::std::string& name, const ::glm::vec3& value) override;
		void SetFloat4(const std::string& name, const ::glm::vec4& value) override;

		void SetMat3(const std::string& name, const glm::mat3& value) override;
		void SetMat4(const std::string& name, const ::glm::mat4& value) override;

		virtual const std::string& GetName() const override { return m_Name; }
		
	private:
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);

		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries();
		void CreateProgram();
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);
	
	private:

		uint32_t m_RendererID;

		std::string m_FilePath;
		// TODO: Use glObjectLabel instead of string member. This allows you to see the name from external opengl debugging tools as well.
		std::string m_Name;

		std::unordered_map<GLenum, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<uint32_t>> m_OpenGLSPIRV;

		std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;
	};
}