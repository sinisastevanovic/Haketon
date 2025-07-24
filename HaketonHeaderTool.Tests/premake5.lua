project "HaketonHeaderTool.Tests"
	kind "ConsoleApp"
	language "C#"
	dotnetframework "net8.0"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.cs"
	}

	links
	{
		"HaketonHeaderTool"
	}

	nuget
	{
		"Microsoft.NET.Test.Sdk:17.8.0",
		"xunit:2.6.6",
		"xunit.runner.visualstudio:2.5.6",
		"FluentAssertions:6.12.0"
	}

	filter "configurations:Debug"
		defines "HK_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "HK_DIST"
		runtime "Release"
		optimize "on"
		symbols "off"