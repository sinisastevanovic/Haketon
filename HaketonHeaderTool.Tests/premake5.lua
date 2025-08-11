project "HaketonHeaderTool.Tests"
	kind "ConsoleApp"
	language "C#"
	dotnetframework "net8.0"

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

	filter "configurations:Debug or configurations:DebugEditor"
		targetdir ("%{prj.location}/bin/Debug")
		objdir ("%{prj.location}/obj/Debug")
		defines "HK_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release or configurations:ReleaseEditor"
		targetdir ("%{prj.location}/bin/Release")
		objdir ("%{prj.location}/obj/Release")
		defines "HK_RELEASE"
		runtime "Release"
		optimize "on"