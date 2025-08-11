HAKETON_ENGINE_ROOT = _SCRIPT_DIR
print("HAKETON_ENGINE_ROOT is: " .. tostring(HAKETON_ENGINE_ROOT))

include (path.join(HAKETON_ENGINE_ROOT, "vendor/premake/premake_customization/solution_items.lua"))

workspace "Haketon"
	architecture "x86_64"
	startproject "HaketonEditor"

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


	configurations
	{
		"Debug",
		"Release",
		"DebugEditor",
		"ReleaseEditor",
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

	include "Engine.lua"