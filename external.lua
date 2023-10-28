-- external.lua
IncludeDir = {}
IncludeDir["mccinfo"]  = "../include"
IncludeDir["wil"] = "../external/wil/include"
IncludeDir["ValveFileVDF"] = "../external/ValveFileVDF"
IncludeDir["krabs"] = "../external/krabsetw"
IncludeDir["compiletimefsm"] = "../external/compile-time-fsm/single-include"
LibraryDir = {}

group "external"
   --include "external/imgui"
group ""

group "tests"
   include "tests/test_mccinfo"
group ""

group "core"
   include "include/"
group ""