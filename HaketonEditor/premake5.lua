print(IncludeDir["ImGui"])

project "HaketonEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{prj.location}/bin/" .. outputdir)
	objdir ("%{prj.location}/intermediate/" .. outputdir)

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/GeneratedFiles/**.cpp"
	}

	includedirs
	{
		"%{IncludeDir.HaketonEngine}",
		"%{IncludeDir.HaketonEditor}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.rttr}",
		"%{IncludeDir.rapidjson}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.fmt}",
	}

	defines { "NOMINMAX" }

	dependson 
	{
		"Haketon"
	}

	links
	{
		"Haketon"
	}

	postbuildcommands
	{
		"{COPY} %{LibraryDir.VulkanSDK_DLL} %{prj.location}/bin/" .. outputdir
	}

	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }

	filter { "configurations:DebugEditor or configurations:ReleaseEditor" }
		defines {"HK_ENGINE_DLL_IMPORT", "RTTR_DLL", "FMT_SHARED", "HK_EDITOR"}

	filter "configurations:DebugEditor"
		libdirs { "%{LibraryDir.RTTRDllLib}/Debug" }
		links { "rttr_core_d" }
		postbuildcommands {
            -- Copy RTTR DLL next to CoreEngine.dll
            ("{COPY} %{LibraryDir.RTTRDllBin}/Debug/rttr_core_d.dll %{cfg.targetdir}"),
			("{COPY} " .. path.join(HAKETON_ENGINE_ROOT, "Haketon/bin/", outputdir, "Haketon.dll") .. " %{cfg.targetdir}")
        }

	filter "configurations:ReleaseEditor"
		libdirs { "%{LibraryDir.RTTRDllLib}/Release" }
		links { "rttr_core" }
		postbuildcommands {
            -- Copy RTTR DLL next to CoreEngine.dll
            ("{COPY} %{LibraryDir.RTTRDllBin}/Release/rttr_core.dll %{cfg.targetdir}"),
			("{COPY} " .. path.join(HAKETON_ENGINE_ROOT, "Haketon/bin/", outputdir, "Haketon.dll") .. " %{cfg.targetdir}")
        }

	filter "configurations:Debug or configurations:DebugEditor"
		defines "HK_DEBUG"
		runtime "Debug"
		symbols "on"
		prebuildcommands
		{
			("dotnet %s/HaketonHeaderTool/bin/Debug/net8.0/HaketonHeaderTool.dll %s HaketonEditor"):format(HAKETON_ENGINE_ROOT, _SCRIPT_DIR),
			--"../scripts/Win-GenProjects.bat"
		}

	filter "configurations:Release or configurations:ReleaseEditor"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			("dotnet %s/HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll %s HaketonEditor"):format(HAKETON_ENGINE_ROOT, _SCRIPT_DIR),
			--"../scripts/Win-GenProjects.bat"
		}

	filter "configurations:Dist"
		defines "HK_DIST"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			("dotnet %s/HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll %s HaketonEditor"):format(HAKETON_ENGINE_ROOT, _SCRIPT_DIR),
			--"../scripts/Win-GenProjects.bat"
		}