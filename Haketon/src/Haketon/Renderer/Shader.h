#pragma once

#include <string>

#include "Haketon/Asset/Asset.h"
#include "Haketon/Math/Math.h"

#include <filesystem>

namespace Haketon {

	CLASS(abstract)
	class HK_API Shader : public Asset
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
		
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const FVec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const FVec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const FVec4& value) = 0;

		virtual void SetMat3(const std::string& name, const glm::mat3& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual AssetType GetType() const override { return AssetType::Shader; }

		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		static Ref<Shader> Create(const std::filesystem::path& filePath);

		RTTR_ENABLE(Asset)
	};
}