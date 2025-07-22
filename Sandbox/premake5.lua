project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.rttr}"
	}

	links
	{
		"Haketon"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "HK_DEBUG"
		runtime "Debug"
		symbols "on"
		prebuildcommands
		{
			"dotnet %{wks.location}HaketonHeaderTool/bin/Debug/net8.0/HaketonHeaderTool.dll %{wks.location} Sandbox"
		}

	filter "configurations:Release"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			"dotnet %{wks.location}HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll %{wks.location} Sandbox"
		}

	filter "configurations:Dist"
		defines "HK_DIST"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			"dotnet %{wks.location}HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll %{wks.location} Sandbox"
		}