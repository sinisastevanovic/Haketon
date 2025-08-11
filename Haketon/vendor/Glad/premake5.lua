project "Glad"
    kind "StaticLib"
    language "C"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("intermediate/" .. outputdir .. "/%{prj.name}")

    files
    {
        "include/glad/glad.h",
        "include/KHR/khrplatform.h",
        "src/glad.c"
    }

	includedirs
	{
		"include"
	}

    filter "system:windows"
        systemversion "latest"
        
    filter "configurations:Debug or configurations:DebugEditor"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release or configurations:ReleaseEditor"
		runtime "Release"
		optimize "on"
