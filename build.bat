@echo off
rem Convenience wrapper around `cmake --preset` / `cmake --build --preset`.
rem
rem Usage:
rem   build.bat                       default: mingw-qt-x64-debug
rem   build.bat <configure-preset>    configure + build that preset
rem   build.bat all                   every preset valid on Windows
rem   build.bat list                  show known presets

setlocal EnableDelayedExpansion

set "DEFAULT_PRESET=mingw-qt-x64-debug"
set "HOST_ALL=vs2022-x64 mingw-qt-x64 mingw-qt-x64-debug"

set "TARGET=%~1"
if "%TARGET%"=="" set "TARGET=%DEFAULT_PRESET%"

if /I "%TARGET%"=="list" (
    echo Configure presets:
    echo   vs2022-win32, vs2022-x64
    echo   mingw-qt-x64, mingw-qt-x64-debug, mingw-qt-x64-ci
    echo   linux-qt-vulkan
    echo.
    echo Host-default 'all' on Windows:
    echo   %HOST_ALL%
    exit /b 0
)

if /I "%TARGET%"=="all" (
    set "FAILED="
    for %%P in (%HOST_ALL%) do (
        call :run_one %%P
        if errorlevel 1 set "FAILED=!FAILED! %%P"
    )
    if defined FAILED (
        echo FAILED:!FAILED! 1>&2
        exit /b 1
    )
    echo All presets built: %HOST_ALL%
    exit /b 0
)

call :run_one %TARGET%
exit /b %ERRORLEVEL%

:run_one
set "CFG=%~1"
call :build_name_for "%CFG%"
if "%BLD%"=="" (
    echo error: unknown configure preset '%CFG%' 1>&2
    exit /b 2
)
echo ==^> [%CFG%] configure
cmake --preset %CFG%
if errorlevel 1 exit /b 1
echo ==^> [%CFG%] build (%BLD%)
cmake --build --preset %BLD%
exit /b %ERRORLEVEL%

:build_name_for
set "BLD="
if /I "%~1"=="vs2022-win32"       set "BLD=build-release"
if /I "%~1"=="vs2022-x64"         set "BLD=build-release-x64"
if /I "%~1"=="mingw-qt-x64"       set "BLD=build-mingw-qt-x64"
if /I "%~1"=="mingw-qt-x64-debug" set "BLD=build-mingw-qt-x64-debug"
if /I "%~1"=="mingw-qt-x64-ci"    set "BLD=build-mingw-qt-x64-ci"
if /I "%~1"=="linux-qt-vulkan"    set "BLD=build-linux-qt-vulkan"
exit /b 0
