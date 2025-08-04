project "HaketonEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
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
		"%{wks.location}/Haketon/vendor/spdlog/include",
		"%{wks.location}/Haketon/src",
		"%{wks.location}/Haketon/vendor",
		"%{wks.location}/HaketonEditor/src",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.rttr}",
		"%{IncludeDir.rapidjson}",
		"%{IncludeDir.ImGuizmo}",
	}

	links
	{
		"Haketon"
	}

	postbuildcommands
	{
		"{COPY} %{LibraryDir.VulkanSDK_DLL} %{prj.location}/bin/" .. outputdir
	}

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }

	filter "configurations:Debug"
		defines "HK_DEBUG"
		runtime "Debug"
		symbols "on"
		prebuildcommands
		{
			"dotnet ../HaketonHeaderTool/bin/Debug/net8.0/HaketonHeaderTool.dll .. HaketonEditor",
			"../scripts/Win-GenProjects.bat"
		}

	filter "configurations:Release"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			"dotnet ../HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll .. HaketonEditor",
			"../scripts/Win-GenProjects.bat"
		}

	filter "configurations:Dist"
		defines "HK_DIST"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			"dotnet ../HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll .. HaketonEditor",
			"../scripts/Win-GenProjects.bat"
		}