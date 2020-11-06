project "rttr"
    kind "StaticLib"
    language "C++"
	cppdialect "C++17"
    staticruntime "on"

    targetdir ("lib/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "include/rttr/**.h",
        "include/rttr/**.cpp"
    }

	includedirs
	{
		"include"
	}

    filter "system:windows"
        systemversion "latest"
        
    filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
