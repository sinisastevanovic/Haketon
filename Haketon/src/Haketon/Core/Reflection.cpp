#include "hkpch.h"
#include "Reflection.h"

#include <rttr/type>
#include <rttr/registration>
#include <glm/glm.hpp>
#include "glm/detail/type_quat.hpp"
#include "Haketon/Math/Math.h"
#include "Haketon/Scene/ScriptRegistry.h"
#include "GeneratedFiles/HaketonComponentSerialization.gen.h"
#include <filesystem>

#include "Haketon/Asset/AssetManager.h"
#include "Serialization/TypeHandlerRegistry.h"

RTTR_REGISTRATION
{
    using namespace rttr;

    registration::class_<glm::vec2>("Vector2")
                .constructor()
                .property("X", &glm::vec2::x)
                .property("Y", &glm::vec2::y);
    
    registration::class_<glm::vec3>("Vector3")
                .constructor()
                .property("X", &glm::vec3::x)
                .property("Y", &glm::vec3::y)
                .property("Z", &glm::vec3::z);

    registration::class_<glm::vec4>("Vector4")
                .constructor()
                .property("R", &glm::vec4::r)
                .property("G", &glm::vec4::g)
                .property("B", &glm::vec4::b)
                .property("A", &glm::vec4::a);

	registration::class_<glm::quat>("Quaternion")
				.constructor()
				.property("X", &glm::quat::x)
				.property("Y", &glm::quat::y)
				.property("Z", &glm::quat::z)
				.property("W", &glm::quat::w);

	registration::class_<glm::ivec2>("IVector2")
				.constructor()
				.property("X", &glm::ivec2::x)
				.property("Y", &glm::ivec2::y);

	registration::class_<glm::ivec3>("IVector3")
				.constructor()
				.property("X", &glm::ivec3::x)
				.property("Y", &glm::ivec3::y)
				.property("Z", &glm::ivec3::z);

	registration::class_<glm::ivec4>("IVector4")
				.constructor()
				.property("X", &glm::ivec4::x)
				.property("Y", &glm::ivec4::y)
				.property("Z", &glm::ivec4::z)
				.property("W", &glm::ivec4::w);

	registration::class_<Haketon::FColor>("FColor")
				.constructor()
				.property("R", &Haketon::FColor::getR, &Haketon::FColor::setR)
				.property("G", &Haketon::FColor::getG, &Haketon::FColor::setG)
				.property("B", &Haketon::FColor::getB, &Haketon::FColor::setB)
				.property("A", &Haketon::FColor::getA, &Haketon::FColor::setA);

	registration::class_<std::filesystem::path>("std::filesystem::path")
				.constructor();
}

#include "GeneratedFiles/AutoReflection.gen.h"

namespace Haketon
{
	void Reflection::Initialize()
	{
		// Call auto-generated registration functions
		RegisterAllHaketonTypes();
		
		// Force RTTR to initialize by accessing the type system
		rttr::type::get_global_properties();
		
		// Initialize script registry after all types are registered
		ScriptRegistry::Get().Initialize();

		RegisterHaketonComponents();

		TypeHandlerRegistry::GetInstance().RegisterHandlers<Haketon::AssetHandle>(
			[](const rttr::variant& v) -> nlohmann::json
			{
				return v.get_value<AssetHandle>().GetUUID().GetValue();
			},
			[](const nlohmann::json& j, rttr::variant& v)
			{
				v = AssetHandle(j.get<uint64_t>());
			}
		);
		TypeHandlerRegistry::GetInstance().RegisterHandlers<Haketon::UUID>(
			[](const rttr::variant& v) -> nlohmann::json
			{
				return v.get_value<UUID>().GetValue();
			},
			[](const nlohmann::json& j, rttr::variant& v)
			{
				v = UUID(j.get<uint64_t>());
			}
		);

		TypeHandlerRegistry::GetInstance().RegisterHandlers<Asset>(
			[](const rttr::variant& v) -> nlohmann::json
			{
				Asset* asset = v.get_value<Asset*>();
				if (asset)
				{
					return asset->GetHandle().GetValue();
				}
				return nullptr;
			},
			[](const nlohmann::json& j, rttr::variant& v)
			{
				if (j.is_null())
				{
					v.clear();
					return;
				}

				AssetHandle handle(j.get<uint64_t>());
				rttr::type assetType = v.get_type();
				auto args = v.get_type().get_template_arguments();
				if (!args.empty())
				{
					assetType = *args.begin();
				}

				auto loader = TypeHandlerRegistry::GetInstance().FindAssetLoader(assetType);
				v = loader(handle);
			}
		);

	}

	void Reflection::Shutdown()
	{
		ScriptRegistry::Get().Shutdown();
		UnregisterHaketonComponents();
	}
}
