include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

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

group "Dependencies"
	include "vendor/premake"
	include "Haketon/vendor/GLFW"
	include "Haketon/vendor/Glad"
	include "Haketon/vendor/imgui"
group ""

include "Haketon"
include "Sandbox"
include "HaketonEditor"
include "HaketonHeaderTool"
include "HaketonHeaderTool.Tests"