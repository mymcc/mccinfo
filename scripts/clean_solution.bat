@echo off
setlocal enabledelayedexpansion

set root="%~dp0..\"
pushd %root%

rem Visual Studio Solution Files
del /q /f /s "*.sln"
del /q /f /s "include\*.vcxproj"
del /q /f /s "include\*.vcxproj.user"
del /q /f /s "include\*packages.config"
del /q /f /s "include\*README.md"
del /q /f /s "tests\test_mccinfo\*.vcxproj"
del /q /f /s "tests\test_mccinfo\*.vcxproj.user"

rem Visual Studio Solution Folders and Binaries
for %%G in (
    .pytest_cache
    .vs 
    bin 
    bin-int
    packages
    tests\bin
    tests\bin-int
) do (
    set "target=%%G"
    rmdir /s /q !target! >nul 2>&1 && (echo Removed folder - %root%!target!) || (echo Could Not Find %root%!target!)
)

echo %cmdcmdline%|find /i """%~f0""">nul && (
    rem Batchfile was double clicked
    pause
) || (
    rem Batchfile was executed from the console
)

popd
endlocal