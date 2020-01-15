workspace "Haketon"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Haketon/vendor/GLFW/include"
IncludeDir["Glad"] = "Haketon/vendor/Glad/include"
IncludeDir["ImGui"] = "Haketon/vendor/imgui2"
IncludeDir["glm"] = "Haketon/vendor/glm"

include "Haketon/vendor/GLFW"
include "Haketon/vendor/Glad"
include "Haketon/vendor/imgui2"

project "Haketon"
	location "Haketon"
	kind "SharedLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "hkpch.h"
	pchsource "Haketon/src/hkpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"HK_PLATFORM_WINDOWS",
			"HK_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines "HK_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "HK_DIST"
		runtime "Release"
		optimize "On"
		

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Haketon/vendor/spdlog/include",
		"Haketon/src",
		"%{IncludeDir.glm}"
	}

	links
	{
		"Haketon"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"HK_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines "HK_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "HK_DIST"
		runtime "Release"
		optimize "On"