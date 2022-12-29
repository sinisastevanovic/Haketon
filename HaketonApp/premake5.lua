project "HaketonApp"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    --targetdir "bin/%{cfg.buildcfg}"
    staticruntime "off"

    files { "Source/**.h", "Source/**.cpp" }

    includedirs
    {
        "../ThirdParty/imgui",
        "../ThirdParty/glfw/include",

        "../Haketon/src",

        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.glm}",
    }

    links
    {
        "Haketon"
    }

    targetdir ("../Binaries/" .. outputdir .. "/%{prj.name}")
    objdir ("../Intermediate/" .. outputdir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "HK_PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        defines { "HK_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "HK_RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"

   filter "configurations:Dist"
        kind "WindowedApp"
        defines { "HK_DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"