-- premake5.lua
include "external/premake/premake_customization/solution_items.lua"

workspace "mccinfo"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "test_mccinfo"

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    solution_items {"premake5.lua", "external.lua"}
    
include "external.lua"