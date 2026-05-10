@echo off
REM build_gnu.bat: Windows MinGW/Clang fallback build entry for NanoDB
REM =====================================================================
REM NanoDB Build Script (MinGW/GCC/Clang) for Windows - Fallback
REM =====================================================================

echo.
echo ====================================================================
echo  NanoDB - GNU/Clang Build Script (Windows fallback)
echo ====================================================================
echo.

:: Detect compiler
where g++ >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set CXX=g++
) else (
    where clang++ >nul 2>&1
    if %ERRORLEVEL% equ 0 (
        set CXX=clang++
    ) else (
        echo Error: No g++ or clang++ found in PATH.
        echo Please install MinGW-w64, MSYS2, or Visual Studio Build Tools.
        echo Recommended: install MSYS2 and run from MSYS2 MinGW shell.
        exit /b 1
    )
)

echo Using compiler: %CXX%

if not exist build mkdir build
if not exist build\obj mkdir build\obj
if not exist bin mkdir bin

set INCLUDE_DIR=-Iinclude
set CXXFLAGS=-std=c++17 -O2 -Wall %INCLUDE_DIR%

echo Compiling src/pager.cpp...
%CXX% %CXXFLAGS% -c src/pager.cpp -o build\obj\pager.o || (echo Failed; exit /b 1)

echo Compiling src/type.cpp...
%CXX% %CXXFLAGS% -c src/type.cpp -o build\obj\type.o || (echo Failed; exit /b 1)

echo Compiling src/test_runner.cpp...
%CXX% %CXXFLAGS% -c src/test_runner.cpp -o build\obj\test_runner.o || (echo Failed; exit /b 1)

echo Linking...
%CXX% build\obj\pager.o build\obj\type.o build\obj\test_runner.o -o bin\nanodb.exe || (echo Link failed; exit /b 1)

echo Build complete. Executable: bin\nanodb.exe
exit /b 0
