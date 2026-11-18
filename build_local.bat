@echo off
rem Genesix by DSK - local Windows build (generic, no personal paths).
rem Usage: build_local.bat [Release^|Debug] [package]
setlocal
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"
if "%GX_DEPS%"=="" set "GX_DEPS=%USERPROFILE%\.gx-deps"
cd /d "%~dp0"

rem Load MSVC x64 environment (VS 2022 / Build Tools) if cl.exe is not on PATH
where cl >nul 2>nul
if not errorlevel 1 goto :haveCl
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul || goto :fail
:haveCl

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=%CONFIG% "-DFETCHCONTENT_BASE_DIR=%GX_DEPS%" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 || goto :fail
cmake --build build || goto :fail
ctest --test-dir build --output-on-failure || goto :fail

if /i "%~2"=="package" (
  python scripts\package.py --platform win --build build --config %CONFIG% || goto :fail
)
echo.
echo VST3:       build\Genesix_artefacts\%CONFIG%\VST3\Genesix by DSK.vst3
echo Standalone: build\Genesix_artefacts\%CONFIG%\Standalone\Genesix by DSK.exe
exit /b 0
:fail
echo BUILD FAILED
exit /b 1
