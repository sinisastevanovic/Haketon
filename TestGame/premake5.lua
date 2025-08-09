HaketonPath = os.getenv("HAKETON_ENGINE_PATH") or "D:/Haketon/"

project "TestGame"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("intermediate/" .. outputdir .. "/%{prj.name}")

    files { "src/**.h", "src/**.cpp" }

    includedirs
    {
        "src",
		HaketonPath .. "/Haketon/vendor/spdlog/include",
		HaketonPath .. "/Haketon/src",
		HaketonPath .. "/Haketon/vendor",
		HaketonPath .. "/Haketon/vendor/glm",
        HaketonPath .. "/Haketon/vendor/entt/include",
		HaketonPath .. "/Haketon/vendor/rttr/include",
    }

    filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }

    filter "configurations:Debug or configurations:Release or configurations:Dist"
        kind "ConsoleApp"
        links { "Haketon" }

    filter "configurations:DebugEditor or configurations:ReleaseEditor"
        kind "SharedLib"
        defines { "GAME_DLL", "HK_ENGINE_DLL_IMPORT" }
        links { "Haketon" }

	filter "configurations:Debug or configurations:DebugEditor"
		defines "HK_DEBUG"
		runtime "Debug"
		symbols "on"
		prebuildcommands
		{
			--"dotnet ../HaketonHeaderTool/bin/Debug/net8.0/HaketonHeaderTool.dll .. TestGame",
			--"../scripts/Win-GenProjects.bat"
		}

	filter "configurations:Release  or configurations:ReleaseEditor"
		defines "HK_RELEASE"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			--"dotnet ../HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll .. TestGame",
			--"../scripts/Win-GenProjects.bat"
		}

	filter "configurations:Dist"
		defines "HK_DIST"
		runtime "Release"
		optimize "on"
		prebuildcommands
		{
			--"dotnet ../HaketonHeaderTool/bin/Release/net8.0/HaketonHeaderTool.dll .. TestGame",
			--"../scripts/Win-GenProjects.bat"
		}