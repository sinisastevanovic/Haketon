project "Haketon"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "hkpch.h"
	pchsource "src/hkpch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/GeneratedFiles/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	includedirs
	{
		"src",
		"vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.rapidjson}",
		"%{IncludeDir.rttr}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.VulkanSDK}"
	}

	libdirs
	{
		"vendor/rttr/lib"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		--"yaml-cpp",
		"opengl32.lib",
		--"librttr_core.lib"
	}

	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

		defines
		{
		}

	filter "configurations:Debug"
		defines "HK_DEBUG"
		runtime "Debug"
		symbols "on"
		prebuildcommands
		{
			"dotnet %{wks.location}HaketonHeaderTool/bin/Debug/net8.0/HaketonHeaderTool.dll %{wks.location} Haketon"
		}
		links
		{
			"librttr_core_d.lib",
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

	filter "configurations:Release"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			"dotnet %{wks.location}HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll %{wks.location} Haketon"
		}
		links
		{
			"librttr_core.lib",
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

	filter "configurations:Dist"
		defines "HK_DIST"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			"dotnet %{wks.location}HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll %{wks.location} Haketon"
		}
		links
		{
			"librttr_core.lib",
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}