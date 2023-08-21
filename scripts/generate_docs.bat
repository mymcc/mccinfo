@echo off
setlocal enabledelayedexpansion

set root="%~dp0..\"
pushd %root%

:: Set the path to Doxygen
set doxygen=".\external\doxygen\doxygen.exe"

:: Check if Doxygen exists at the specified path
if not exist %doxygen% (
    echo doxygen not found at %doxygen%
    exit /b 1
)

:: Symlink the repo README.md
copy /y README.md include\README.md

:: Generate Docs
%doxygen% Doxyfile

popd
endlocal