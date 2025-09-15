#include "hkpch.h"
#include "Material.h"
#include "Haketon/Core/Log.h"

namespace Haketon {
	
	void MaterialProperty::Serialize(ISerializer* serializer) const
	{
		std::visit([&](const auto& value) {
			using T = std::decay_t<decltype(value)>;
			
			if constexpr (std::is_same_v<T, float>)
				serializer->Serialize(serializer->GetCurrentMemberName(), value);
			else if constexpr (std::is_same_v<T, int>)
				serializer->Serialize(serializer->GetCurrentMemberName(), value);
			else if constexpr (std::is_same_v<T, bool>)
				serializer->Serialize(serializer->GetCurrentMemberName(), value);
			else if constexpr (std::is_same_v<T, FVec2>)
				serializer->SerializeValue(serializer->GetCurrentMemberName(), value);
			else if constexpr (std::is_same_v<T, FVec3>)
				serializer->SerializeValue(serializer->GetCurrentMemberName(), value);
			else if constexpr (std::is_same_v<T, FVec4>)
				serializer->SerializeValue(serializer->GetCurrentMemberName(), value);
			else if constexpr (std::is_same_v<T, glm::mat3>)
				serializer->SerializeValue(serializer->GetCurrentMemberName(), value);
			else if constexpr (std::is_same_v<T, glm::mat4>)
				serializer->SerializeValue(serializer->GetCurrentMemberName(), value);
			else if constexpr (std::is_same_v<T, Ref<Texture2D>>)
			{
				if (value)
				{
					serializer->Serialize(serializer->GetCurrentMemberName(), value->GetHandle().GetValue());
				}
			}
		}, Value);
	}

	void MaterialProperty::Deserialize(IDeserializer* deserializer)
	{
	}

	Material::Material(const Ref<Shader>& shader)
		: m_Shader(shader)
	{
	}

	void Material::Bind() const
	{
		if (!m_Shader)
		{
			HK_CORE_WARN("Material::Bind - No shader assigned to material!");
			return;
		}

		m_Shader->Bind();
		ApplyProperties();
	}

	void Material::Unbind() const
	{
		if (m_Shader)
			m_Shader->Unbind();
	}

	void Material::SetFloat(const std::string& name, float value)
	{
		m_Properties[name] = MaterialProperty(value);
		SetDirty(true);
	}

	void Material::SetInt(const std::string& name, int value)
	{
		m_Properties[name] = MaterialProperty(value);
		SetDirty(true);
	}

	void Material::SetBool(const std::string& name, bool value)
	{
		m_Properties[name] = MaterialProperty(value);
		SetDirty(true);
	}

	void Material::SetFloat2(const std::string& name, const FVec2& value)
	{
		m_Properties[name] = MaterialProperty(value);
		SetDirty(true);
	}

	void Material::SetFloat3(const std::string& name, const FVec3& value)
	{
		m_Properties[name] = MaterialProperty(value);
		SetDirty(true);
	}

	void Material::SetFloat4(const std::string& name, const FVec4& value)
	{
		m_Properties[name] = MaterialProperty(value);
		SetDirty(true);
	}

	void Material::SetMat3(const std::string& name, const glm::mat3& value)
	{
		m_Properties[name] = MaterialProperty(value);
		SetDirty(true);
	}

	void Material::SetMat4(const std::string& name, const glm::mat4& value)
	{
		m_Properties[name] = MaterialProperty(value);
		SetDirty(true);
	}

	void Material::SetTexture(const std::string& name, const Ref<Texture2D>& texture)
	{
		m_Properties[name] = MaterialProperty(texture);
		SetDirty(true);
	}

	void Material::ApplyProperties() const
	{
		if (!m_Shader)
			return;

		int textureSlot = 0;
		for (const auto& [name, property] : m_Properties)
		{
			std::visit([&](const auto& value) {
				using T = std::decay_t<decltype(value)>;
				
				if constexpr (std::is_same_v<T, float>)
					m_Shader->SetFloat(name, value);
				else if constexpr (std::is_same_v<T, int>)
					m_Shader->SetInt(name, value);
				else if constexpr (std::is_same_v<T, bool>)
					m_Shader->SetInt(name, value ? 1 : 0);
				else if constexpr (std::is_same_v<T, FVec2>)
					m_Shader->SetFloat2(name, value);
				else if constexpr (std::is_same_v<T, FVec3>)
					m_Shader->SetFloat3(name, value);
				else if constexpr (std::is_same_v<T, FVec4>)
					m_Shader->SetFloat4(name, value);
				else if constexpr (std::is_same_v<T, glm::mat3>)
					m_Shader->SetMat3(name, value);
				else if constexpr (std::is_same_v<T, glm::mat4>)
					m_Shader->SetMat4(name, value);
				else if constexpr (std::is_same_v<T, Ref<Texture2D>>)
				{
					if (value)
					{
						value->Bind(textureSlot);
						m_Shader->SetInt(name, textureSlot);
						textureSlot++;
					}
				}
			}, property.Value);
		}
	}

	Ref<Material> Material::Create(const Ref<Shader>& shader)
	{
		return CreateRef<Material>(shader);
	}

	Ref<Material> Material::Create(const std::filesystem::path& filePath)
	{
		// TODO: Implement material loading from file
		HK_CORE_ERROR("Material::Create from file not implemented yet!");
		return nullptr;
	}
}

RTTR_REGISTRATION
{
	rttr::registration::class_<Haketon::Material>("Material")
		.property("m_Shader", &Haketon::Material::m_Shader)
		.property("m_Properties", &Haketon::Material::m_Properties);
}