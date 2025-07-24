#include "hkpch.h"
#include "Reflection.h"

#include <rttr/type>
#include <rttr/registration>
#include <glm/glm.hpp>
#include "glm/detail/type_quat.hpp"

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
	}
}