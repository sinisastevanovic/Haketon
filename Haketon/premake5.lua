project "Haketon"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    --targetdir "bin/%{cfg.buildcfg}"
    staticruntime "off"

    files { "Source/**.h", "Source/**.cpp" }

    includedirs
    {
        "Source",

        "../ThirdParty/imgui",
        "../ThirdParty/glfw/include",
        "../ThirdParty/stb_image",
        "../ThirdParty/glm",

        "%{IncludeDir.VulkanSDK}",
    }

    links
    {
        "ImGui",
        "GLFW",

        "%{Library.Vulkan}",
    }

    targetdir ("Binaries/" .. outputdir .. "/%{prj.name}")
    objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

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