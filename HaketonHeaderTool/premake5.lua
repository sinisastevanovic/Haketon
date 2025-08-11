project "HaketonHeaderTool"
    kind "ConsoleApp"
    language "C#"
    dotnetframework "net8.0"

    files
	{
		"src/**.cs"
	}

	filter "system:windows"
		systemversion "latest"

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

require "vstudio"

local function platformsElement(cfg)
  _p(2,'<Platforms>x64</Platforms>')
end

premake.override(premake.vstudio.cs2005.elements, "projectProperties", function (oldfn, cfg)
  return table.join(oldfn(cfg), {
    platformsElement,
  })
end)