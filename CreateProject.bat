@echo off

set /p "project_name=Enter Project Name: "
if not exist "%~dp0%project_name%\Source" mkdir %~dp0%project_name%\Source
xcopy /s /f /y %~dp0ThirdParty\premake5\ProjectSetup\Project.cpp %~dp0%project_name%\Source\%project_name%.cpp*
:: Create project premake5.lua
break>%project_name%\premake5.lua
(
echo project "%project_name%"
echo    kind "ConsoleApp"
echo    language "C++"
echo    cppdialect "C++17"
echo    --targetdir "bin/%%{cfg.buildcfg}"
echo    staticruntime "off"
echo:
echo    files { "Source/**.h", "Source/**.cpp" }
echo:
echo    includedirs
echo    {
echo        "../ThirdParty/imgui",
echo        "../ThirdParty/glfw/include",
echo:
echo        "../Haketon/Source",
echo:
echo        "%%{IncludeDir.VulkanSDK}",
echo        "%%{IncludeDir.glm}",
echo    }
echo:
echo    links
echo    {
echo        "Haketon"
echo    }
echo:
echo    targetdir ^("../Binaries/" .. outputdir .. "/%%{prj.name}"^)
echo    objdir ^("../Intermediate/" .. outputdir .. "/%%{prj.name}"^)
echo:
echo    filter "system:windows"
echo        systemversion "latest"
echo        defines { "HK_PLATFORM_WINDOWS" }
echo:
echo    filter "configurations:Debug"
echo        defines { "HK_DEBUG" }
echo        runtime "Debug"
echo        symbols "On"
echo:
echo    filter "configurations:Release"
echo        defines { "HK_RELEASE" }
echo        runtime "Release"
echo        optimize "On"
echo        symbols "On"
echo:
echo   filter "configurations:Dist"
echo        kind "WindowedApp"
echo        defines { "HK_DIST" }
echo        runtime "Release"
echo        optimize "On"
echo        symbols "Off"
)>%project_name%\premake5.lua

:: Create base premake5.lua file
break>premake5.lua
(
    echo workspace "%project_name%"
    echo    architecture "x64"
    echo    configurations { "Debug", "Release", "Dist" }
    echo    startproject "%project_name%"
    echo:
    echo outputdir = "%%{cfg.system}-%%{cfg.architecture}-%%{cfg.buildcfg}"
    echo:
    echo include "HaketonExternal.lua"
    echo include "%project_name%"
)>premake5.lua

xcopy /s /f /y %~dp0ThirdParty\premake5\GLFWpremake5.lua %~dp0ThirdParty\GLFW\premake5.lua*
xcopy /s /f /y %~dp0ThirdParty\premake5\imguipremake5.lua %~dp0ThirdParty\imgui\premake5.lua*
ThirdParty\premake5\premake5.exe vs2022
pause