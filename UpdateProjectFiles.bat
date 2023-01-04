@echo off
pushd ..
if not exist Haketon\ThirdParty\GLFW\premake5.lua xcopy /s /f /y Haketon\ThirdParty\premake5\GLFWpremake5.lua Haketon\ThirdParty\GLFW\premake5.lua*
if not exist Haketon\ThirdParty\imgui\premake5.lua xcopy /s /f /y Haketon\ThirdParty\premake5\imguipremake5.lua Haketon\ThirdParty\imgui\premake5.lua*
Haketon\ThirdParty\premake5\premake5.exe vs2022
popd
pause