include "./vendor/premake/premake_customization/solution_items.lua"

workspace "Haketon"
	architecture "x86_64"
	startproject "HaketonEditor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Haketon/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Haketon/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Haketon/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/Haketon/vendor/glm"
IncludeDir["stb_image"] = "%{wks.location}/Haketon/vendor/stb_image"
IncludeDir["entt"] = "%{wks.location}/Haketon/vendor/entt/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/Haketon/vendor/yaml-cpp/include"
IncludeDir["rttr"] = "%{wks.location}/Haketon/vendor/rttr/include"

group "Dependencies"
	include "vendor/premake"
	include "Haketon/vendor/GLFW"
	include "Haketon/vendor/Glad"
	include "Haketon/vendor/imgui"
	include "Haketon/vendor/yaml-cpp"
group ""

include "Haketon"
include "Sandbox"
include "HaketonEditor"
include "HaketonHeaderTool"