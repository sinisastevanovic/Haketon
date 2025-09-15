#pragma once

#include "Haketon/Core/Core.h"
#include "Haketon/Asset/Asset.h"
#include "Haketon/Renderer/Shader.h"
#include "Haketon/Renderer/Texture.h"
#include <glm/glm.hpp>
#include <variant>
#include <unordered_map>
#include <string>

#include <rttr/registration_friend>

namespace Haketon {

	using MaterialPropertyValue = std::variant<float, int, bool, FVec2, FVec3, FVec4, glm::mat3, glm::mat4, Ref<Texture2D>>;

	STRUCT()
	struct MaterialProperty
	{
		PROPERTY()
		MaterialPropertyValue Value;
		
		MaterialProperty() = default;
		MaterialProperty(const MaterialPropertyValue& value)
			: Value(value) {}
		
		template<typename T>
		T GetValue() const { return std::get<T>(Value); }
		
		template<typename T>
		void SetValue(const T& value) { Value = value; }

		FUNCTION()
		void Serialize(ISerializer* serializer) const;

		FUNCTION()
		void Deserialize(IDeserializer* deserializer);
	};

	class HK_API Material : public Asset
	{
	public:
		Material() = default;
		Material(const Ref<Shader>& shader);
		virtual ~Material() = default;

		void Bind() const;
		void Unbind() const;

		void SetShader(const Ref<Shader>& shader) { m_Shader = shader; SetDirty(true); }
		Ref<Shader> GetShader() const { return m_Shader; }

		void SetFloat(const std::string& name, float value);
		void SetInt(const std::string& name, int value);
		void SetBool(const std::string& name, bool value);
		void SetFloat2(const std::string& name, const FVec2& value);
		void SetFloat3(const std::string& name, const FVec3& value);
		void SetFloat4(const std::string& name, const FVec4& value);
		void SetMat3(const std::string& name, const glm::mat3& value);
		void SetMat4(const std::string& name, const glm::mat4& value);
		void SetTexture(const std::string& name, const Ref<Texture2D>& texture);

		template<typename T>
		T GetProperty(const std::string& name) const
		{
			auto it = m_Properties.find(name);
			if (it != m_Properties.end())
				return it->second.GetValue<T>();
			return T{};
		}

		bool HasProperty(const std::string& name) const { return m_Properties.find(name) != m_Properties.end(); }
		const std::unordered_map<std::string, MaterialProperty>& GetProperties() const { return m_Properties; }

		virtual AssetType GetType() const override { return AssetType::Material; }

		static Ref<Material> Create(const Ref<Shader>& shader);
		static Ref<Material> Create(const std::filesystem::path& filePath);

	private:
		void ApplyProperties() const;
		
	private:
		Ref<Shader> m_Shader;
		std::unordered_map<std::string, MaterialProperty> m_Properties;

		RTTR_ENABLE(Asset)
		RTTR_REGISTRATION_FRIEND
	};
}