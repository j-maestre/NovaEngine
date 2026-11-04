@echo off
rem ============================================================================
rem  NovaEngine - setup de dependencias
rem
rem  Uso:  setup.bat            (compila Debug y Release)
rem        setup.bat Debug      (solo una configuracion)
rem ============================================================================
setlocal EnableExtensions EnableDelayedExpansion

cd /d "%~dp0"
set "ROOT=%CD%"
set "EXT=%ROOT%\deps\external"
set "BUILD=%ROOT%\deps\build"
set "LIBOUT=%ROOT%\deps\lib"
set "BINOUT=%ROOT%\deps\bin"

set "CONFIGS=Debug Release"
if not "%~1"=="" set "CONFIGS=%~1"

rem ---------------------------------------------------------------- git
where git >nul 2>&1
if errorlevel 1 (
    echo [ERROR] git no esta en el PATH. Instalalo desde https://git-scm.com
    goto :fail
)
if not exist "%ROOT%\.git" (
    echo [ERROR] Esta carpeta no es un repositorio git.
    echo         Si lo bajaste como ZIP, clonalo con:
    echo         git clone --recursive https://github.com/j-maestre/NovaEngine
    goto :fail
)

rem ------------------------------------------------- entorno de Visual Studio
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo [ERROR] No se encuentra Visual Studio - falta vswhere.exe.
    goto :fail
)
set "VSPATH="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
if not defined VSPATH (
    echo [ERROR] Visual Studio no tiene instalado "Desarrollo para el escritorio con C++".
    goto :fail
)
echo Visual Studio: %VSPATH%
call "%VSPATH%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -no_logo
if errorlevel 1 goto :fail
cd /d "%ROOT%"

where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake no encontrado. Instala el componente
    echo         "Herramientas de CMake de C++ para Windows" desde el Visual Studio Installer.
    goto :fail
)

rem ---------------------------------------------------------- submodulos
echo.
echo ===== Submodulos =====
git submodule update --init --recursive
if errorlevel 1 goto :fail

rem -------------------------------------------------------- dependencias
call :build assimp "%EXT%\assimp" assimp "-DBUILD_SHARED_LIBS=ON -DASSIMP_BUILD_TESTS=OFF -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DASSIMP_BUILD_SAMPLES=OFF -DASSIMP_INSTALL=OFF -DASSIMP_WARNINGS_AS_ERRORS=OFF"
if errorlevel 1 goto :fail

call :build rapidyaml "%EXT%\rapidyaml" ryml "-DRYML_INSTALL=OFF"
if errorlevel 1 goto :fail

call :build DirectXTK "%EXT%\DirectXTK" DirectXTK "-DBUILD_TOOLS=OFF -DBUILD_XAUDIO_WIN8=OFF -DBUILD_SHARED_LIBS=OFF"
if errorlevel 1 goto :fail

rem ------------------------------------------------- copiar libs y dlls
echo.
echo ===== Copiando librerias =====
for %%c in (%CONFIGS%) do (
    call :collect %%c
    if errorlevel 1 goto :fail
)

echo.
echo ============================================================
echo  Listo. Abre NovaEngine.sln, elige x64 y compila.
echo ============================================================
endlocal
exit /b 0

:fail
echo.
echo ============================================================
echo  [ERROR] El setup ha fallado. Revisa el mensaje de arriba.
echo ============================================================
endlocal
exit /b 1


rem ============================================================================
rem  :build  nombre  carpeta_fuente  target  "opciones cmake"
rem ============================================================================
:build
echo.
echo ===== %~1 =====
cmake -S "%~2" -B "%BUILD%\%~1" -A x64 %~4
if errorlevel 1 exit /b 1
for %%c in (%CONFIGS%) do (
    echo --- %~1 %%c ---
    cmake --build "%BUILD%\%~1" --config %%c --target %~3 --parallel
    if errorlevel 1 exit /b 1
)
exit /b 0


rem ============================================================================
rem  :collect  config
rem ============================================================================
:collect
set "CFG=%~1"
if not exist "%LIBOUT%\%CFG%" mkdir "%LIBOUT%\%CFG%"
if not exist "%BINOUT%\%CFG%" mkdir "%BINOUT%\%CFG%"
del /q "%BINOUT%\%CFG%\*.dll" >nul 2>&1

call :find_lib "assimp-*.lib" "%BUILD%\assimp"
if not defined F ( echo [ERROR] No se encuentra la .lib de assimp en %CFG% & exit /b 1 )
copy /Y "!F!" "%LIBOUT%\%CFG%\assimp.lib" >nul
echo   assimp.lib     ^<- !F!

set "DLLS=0"
for /r "%BUILD%\assimp\bin\%CFG%" %%f in (assimp-*.dll) do (
    copy /Y "%%f" "%BINOUT%\%CFG%\" >nul
    echo   %%~nxf ^<- %%f
    set "DLLS=1"
)
if "!DLLS!"=="0" ( echo [ERROR] No se encuentra la .dll de assimp en %CFG% & exit /b 1 )

call :find_lib "ryml.lib" "%BUILD%\rapidyaml"
if not defined F ( echo [ERROR] No se encuentra ryml.lib en %CFG% & exit /b 1 )
copy /Y "!F!" "%LIBOUT%\%CFG%\ryml.lib" >nul
echo   ryml.lib       ^<- !F!

call :find_lib "DirectXTK.lib" "%BUILD%\DirectXTK"
if not defined F ( echo [ERROR] No se encuentra DirectXTK.lib en %CFG% & exit /b 1 )
copy /Y "!F!" "%LIBOUT%\%CFG%\DirectXTK.lib" >nul
echo   DirectXTK.lib  ^<- !F!
exit /b 0


rem ============================================================================
rem  :find_lib  patron  carpeta   ->  F = ruta del archivo dentro de ...\%CFG%\
rem ============================================================================
:find_lib
set "F="
for /r "%~2" %%f in (%~1) do (
    if exist "%%f" (
        for %%d in ("%%~dpf.") do if /i "%%~nxd"=="%CFG%" set "F=%%f"
    )
)
exit /b 0