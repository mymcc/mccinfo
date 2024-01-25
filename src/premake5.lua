project "mccinfo-monitor"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    staticruntime "on"

    pchheader "pch.h"
    pchsource "pch.cpp"
    
    files 
    {
        "premake5.lua",
        "**.h",
        "**.hpp",
        "**.cpp",
    }

    includedirs
    {
        ".",
        "%{IncludeDir.mccinfo}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.GLEW}",
        "%{IncludeDir.stb}",
    }

    links
    {
        "ImGui",
        "mccinfo",
        "opengl32.lib",
    }

    libdirs
    {

    }

    defines
    {
        "GLEW_STATIC"
    }

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "MCCINFO_MONITOR_PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        kind "ConsoleApp"
        defines { "MCCINFO_MONITOR_DEBUG" }
        runtime "Debug"
        optimize "Off"
        symbols "On"

    filter "configurations:Release"
        kind "WindowedApp"
        defines { "MCCINFO_MONITOR_RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "Off"