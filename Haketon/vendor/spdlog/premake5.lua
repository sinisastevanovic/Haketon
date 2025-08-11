project "spdlog"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir (path.join(HAKETON_ENGINE_ROOT, "bin/" .. outputdir .. "/%{prj.name}"))
    objdir (path.join(HAKETON_ENGINE_ROOT, "intermediate/" .. outputdir .. "/%{prj.name}"))

    files { "src/**.cpp", "include/spdlog/**.h" }
    includedirs { "include", "%{IncludeDir.fmt}" }

    defines { "SPDLOG_COMPILED_LIB", "SPDLOG_FMT_EXTERNAL" }

    dependson { "fmt" }

    filter "files:src/**.cpp"
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