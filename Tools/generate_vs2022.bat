@echo off

SET ScriptPath=%~dp0
SET RootPath=%ScriptPath%..\
SET BuildPath=%ScriptPath%..\intermediate\vs2022\
SET CMakeExePath=%ScriptPath%Build\cmake\windows\bin\cmake.exe

echo Generating %RootPath%
echo %CMakeExePath% -S %RootPath% -B %BuildPath%

%CMakeExePath% -S %RootPath% -B %BuildPath% -G "Visual Studio 17 2022"
