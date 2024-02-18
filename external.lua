-- external.lua
IncludeDir = {}
IncludeDir["mccinfo"]  = "../include"
IncludeDir["wil"] = "../external/wil/include"
IncludeDir["ValveFileVDF"] = "../external/ValveFileVDF"
IncludeDir["krabs"] = "../external/krabsetw"
IncludeDir["compiletimefsm"] = "../external/compile-time-fsm/src"
IncludeDir["lockfree"] = "../external/lockfree"
IncludeDir["sml"] = "../external/sml/include"
IncludeDir["cometa"] = "../external/cometa/include"
IncludeDir["frozen"] = "../external/frozen/include"
IncludeDir["imgui"]  = "../external/imgui"
IncludeDir["GLEW"] = "../external/glew/include"
IncludeDir["spdlog"] = "../external/spdlog/include"
LibraryDir = {}

group "external"
   include "external/imgui"
   include "external/GLEW"
group ""

group "tests"
   include "tests/test_mccinfo"
group ""

group "core"
   include "include/"
   include "src/"
group ""