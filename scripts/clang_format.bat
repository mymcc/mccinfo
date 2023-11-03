@echo off
setlocal enabledelayedexpansion

set root="%~dp0..\"
pushd %root%

:: Set the path to clang-format
set clang_format=".\external\clang\clang-format-10_windows-amd64.exe"

:: Check if clang-format exists at the specified path
if not exist %clang_format% (
    echo clang-format not found at %clang_format%
    exit /b 1
)

set "td=tests"
set "sd=include"

echo Formatting files in %td% and subdirectories
for /r %td% %%f in (*.cpp *.hpp) do (
    echo %%f
    "%clang_format%" -i "%%f"
)

echo Formatting files in %sd% and subdirectories
for /r %sd% %%f in (*.cpp *.hpp) do (
    echo Formatting ... %%f
    "%clang_format%" -i "%%f"
)

echo Done formatting!

popd
endlocal