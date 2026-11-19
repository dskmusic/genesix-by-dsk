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

if /i not "%~2"=="package" goto :done
for /f "usebackq delims=" %%v in (`python scripts\version.py`) do set "VER=%%v"
python scripts\package.py --platform win --build build --config %CONFIG% --version %VER% || goto :fail
set "ISCC=%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe"
if not exist "%ISCC%" set "ISCC=%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe"
if not exist "%ISCC%" goto :noInno
"%ISCC%" /Q "/DAppVersion=%VER%" "/DArtDir=..\build\Genesix_artefacts\%CONFIG%" installer\genesix.iss || goto :fail
python scripts\package.py --platform win --build build --checksums-only || goto :fail
goto :done
:noInno
echo (Inno Setup 6 no encontrado: se omite el instalador .exe)
:done
echo.
echo VST3:       build\Genesix_artefacts\%CONFIG%\VST3\Genesix by DSK.vst3
echo Standalone: build\Genesix_artefacts\%CONFIG%\Standalone\Genesix by DSK.exe
exit /b 0
:fail
echo BUILD FAILED
exit /b 1
