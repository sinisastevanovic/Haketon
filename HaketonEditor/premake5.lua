project "HaketonEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
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
			"%{wks.location}HaketonHeaderTool/bin/Debug/netcoreapp5.0/HaketonHeaderTool.exe %{wks.location} Haketon",
			"%{wks.location}HaketonHeaderTool/bin/Debug/netcoreapp5.0/HaketonHeaderTool.exe %{wks.location} HaketonEditor",
			"%{wks.location}scripts/Win-GenProjects.bat"
		}

	filter "configurations:Release"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			"%{wks.location}HaketonHeaderTool/bin/Release/netcoreapp5.0/HaketonHeaderTool.exe %{wks.location} Haketon",
			"%{wks.location}HaketonHeaderTool/bin/Release/netcoreapp5.0/HaketonHeaderTool.exe %{wks.location} HaketonEditor",
			"%{wks.location}scripts/Win-GenProjects.bat"
		}

	filter "configurations:Dist"
		defines "HK_DIST"
		runtime "Release"
		optimize "on"