project "HaketonEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

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
		"%{IncludeDir.ImGuizmo}",
	}

	links
	{
		"Haketon"
	}

	postbuildcommands
	{
		"{COPY} %{LibraryDir.VulkanSDK_DLL} %{wks.location}/bin/" .. outputdir .. "/%{prj.name}"
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
			"dotnet %{wks.location}HaketonHeaderTool/bin/Debug/net8.0/HaketonHeaderTool.dll %{wks.location} HaketonEditor",
			"%{wks.location}scripts/Win-GenProjects.bat"
		}

	filter "configurations:Release"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			"dotnet %{wks.location}HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll %{wks.location} HaketonEditor",
			"%{wks.location}scripts/Win-GenProjects.bat"
		}

	filter "configurations:Dist"
		defines "HK_DIST"
		runtime "Release"
		optimize "on"