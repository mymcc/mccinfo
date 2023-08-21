-- external.lua
IncludeDir = {}
IncludeDir["mccinfo"]  = "../include"
IncludeDir["wil"] = "../external/wil/include"
IncludeDir["ValveFileVDF"] = "../external/ValveFileVDF"

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