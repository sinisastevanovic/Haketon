@echo off

xcopy /s /f /y %~dp0ThirdParty\premake5\GLFWpremake5.lua %~dp0ThirdParty\GLFW\premake5.lua*
xcopy /s /f /y %~dp0ThirdParty\premake5\imguipremake5.lua %~dp0ThirdParty\imgui\premake5.lua*
ThirdParty\premake5\premake5.exe vs2022
pause