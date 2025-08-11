project "fmt"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir (path.join(HAKETON_ENGINE_ROOT, "bin/" .. outputdir .. "/%{prj.name}"))
	objdir (path.join(HAKETON_ENGINE_ROOT, "intermediate/" .. outputdir .. "/%{prj.name}"))


	files { "src/format.cc", "src/os.cc", "include/fmt/**.h" }
	includedirs { "include" }

	-- Use header-only mode to avoid compilation issues
	defines 
	{ 
		"_CRT_SECURE_NO_WARNINGS",
		"NOMINMAX",
		"WIN32_LEAN_AND_MEAN"
	}

	filter "files:src/**.cc"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"
        buildoptions { "/utf-8" }

	filter "configurations:Debug or configurations:DebugEditor"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release or configurations:ReleaseEditor"
		runtime "Release"
		optimize "on"

	filter "configurations:DebugEditor or configurations:ReleaseEditor"
		defines { "FMT_LIB_EXPORT" }