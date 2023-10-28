project "mccinfo"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    staticruntime "on"

    files 
    {
        "premake5.lua",
        "**.h",
        "**.hpp",
        "**.cpp"
    }

    includedirs
    {
        ".",
        "%{IncludeDir.wil}",
        "%{IncludeDir.ValveFileVDF}",
        "%{IncludeDir.krabs}",
        "%{IncludeDir.compiletimefsm}",
        "%{IncludeDir.lockfree}"
    }

    links
    {

    }

    libdirs
    {
        
    }

    defines
    {
        
    }

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")
   
    filter "system:windows"
        systemversion "latest"
        defines { "MCCINFO_PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        defines { "MCCINFO_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "MCCINFO_RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"