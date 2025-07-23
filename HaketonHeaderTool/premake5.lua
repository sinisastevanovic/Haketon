project "HaketonHeaderTool"
    kind "ConsoleApp"
    language "C#"
    dotnetframework "net8.0"

    files
	{
		"Exceptions.cs",
		"HeaderParser.cs",
		"Logger.cs",
		"Program.cs",
		"SourcePosition.cs"
	}

	filter "system:windows"
		systemversion "latest"

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