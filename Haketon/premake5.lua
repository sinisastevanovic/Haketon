project "Haketon"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{prj.location}/bin/" .. outputdir)
	objdir ("%{prj.location}/intermediate/" .. outputdir)

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
		"%{IncludeDir.spdlog}/**.h",
		"%{IncludeDir.fmt}/**.h",
	}

	defines
	{
		"SPDLOG_COMPILED_LIB",
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"NOMINMAX",
	}

	includedirs
	{
		"src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.rapidjson}",
		"%{IncludeDir.rttr}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.fmt}",
		"%{IncludeDir.VulkanSDK}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib",
		"spdlog",
		"fmt"
	}

	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }

		defines
		{
		}

	filter "configurations:Debug"
		libdirs { "%{LibraryDir.RTTRStatic}/Debug" }
		links { "librttr_core_d" }

	filter "configurations:DebugEditor"
		libdirs { "%{LibraryDir.RTTRDllLib}/Debug" }
		links { "rttr_core_d" }
		postbuildcommands {
            -- Copy RTTR DLL next to CoreEngine.dll
            ("{COPY} %{LibraryDir.RTTRDllBin}/Debug/rttr_core_d.dll %{cfg.targetdir}")
        }

	filter "configurations:ReleaseEditor"
		libdirs { "%{LibraryDir.RTTRDllLib}/Release" }
		links { "rttr_core" }
		postbuildcommands {
            -- Copy RTTR DLL next to CoreEngine.dll
            ("{COPY} %{LibraryDir.RTTRDllBin}/Release/rttr_core.dll %{cfg.targetdir}")
        }

	filter "configurations:Release or configurations:Dist"
		libdirs { "%{LibraryDir.RTTRStatic}/Release" }
		links { "librttr_core" }

	filter "configurations:Debug or configurations:Release or configurations:Dist"
        kind "StaticLib"

    filter "configurations:DebugEditor or configurations:ReleaseEditor"
        kind "SharedLib"
        defines { "HK_ENGINE_DLL", "RTTR_DLL", "FMT_SHARED", "HK_EDITOR" }
		

	filter "configurations:Debug or configurations:DebugEditor"
		defines "HK_DEBUG"
		runtime "Debug"
		symbols "on"
		prebuildcommands
		{
			("dotnet %s/HaketonHeaderTool/bin/Debug/net8.0/HaketonHeaderTool.dll %s Haketon"):format(HAKETON_ENGINE_ROOT, _SCRIPT_DIR),
		}
		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

	filter "configurations:Release or configurations:ReleaseEditor"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			("dotnet %s/HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll %s Haketon"):format(HAKETON_ENGINE_ROOT, _SCRIPT_DIR),
		}
		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}