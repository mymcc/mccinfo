project "test_mccinfo"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    staticruntime "on"

    files 
    {
        "premake5.lua",
        "**.cpp",
    }

    includedirs
    {
        ".",
        "../%{IncludeDir.mccinfo}",
        "../%{IncludeDir.wil}",
        "../%{IncludeDir.ValveFileVDF}",
        "../%{IncludeDir.krabs}",
        "../%{IncludeDir.compiletimefsm}",
        "../%{IncludeDir.lockfree}"
    }

    links
    {
        "mccinfo",
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
        defines { "MCCINFO_TEST_PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        defines { "MCCINFO_TEST_DEBUG" }
        runtime "Debug"
        optimize "Off"
        symbols "On"

    filter "configurations:Release"
        defines { "MCCINFO_TEST_RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "Off"