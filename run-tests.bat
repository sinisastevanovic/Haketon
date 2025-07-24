@echo off
echo Running HeaderTool Unit Tests...

if not exist "bin\Debug-windows-x86_64\HaketonHeaderTool.Tests" (
    echo Test project not built. Building solution first...
    call scripts\Win-GenProjects.bat
    msbuild Haketon.sln /p:Configuration=Debug /p:Platform=x64
)

cd HaketonHeaderTool.Tests

dotnet test --logger "console;verbosity=normal"

if %ERRORLEVEL% NEQ 0 (
    echo Tests failed!
    pause
    exit /b %ERRORLEVEL%
)

echo All tests passed!
pause