#pragma once

#include <string>
#include <glm/glm.hpp>

namespace Haketon {

	class Shader
	{
	public:
		virtual ~Shader() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
		
		static Shader* Create(const std::string& vertexSrc, const std::string& fragmentSrc);
	};
}