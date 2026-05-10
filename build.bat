@echo off
REM build.bat: Windows MSVC build entry for NanoDB (single-command local build)
REM =====================================================================
REM NanoDB Build Script for Windows (MSVC)
REM =====================================================================

setlocal enabledelayedexpansion

echo.
echo ====================================================================
echo  NanoDB - Database Engine Build Script
echo ====================================================================
echo.

REM Check for Visual Studio
where cl.exe >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Error: MSVC compiler ^(cl.exe^) not found in PATH
    echo Please install Visual Studio or run from Developer Command Prompt
    pause
    exit /b 1
)

echo [*] Found MSVC compiler
cl.exe 2>&1 | findstr /C:"Compiler Version"

REM Create build directories
if not exist build mkdir build
if not exist build\obj mkdir build\obj
if not exist bin mkdir bin

echo [*] Created build directories

REM Compile source files
echo.
echo [*] Compiling source files...

set CXXFLAGS=/std:c++17 /W3 /EHsc /O2 /Iinclude

echo   - Compiling pager.cpp...
cl.exe %CXXFLAGS% /c src/pager.cpp /Fobuild/obj/pager.obj 2>&1 | findstr /C:"error" && (
    echo Error: Failed to compile pager.cpp
    exit /b 1
) || echo     [OK] pager.obj

echo   - Compiling type.cpp...
cl.exe %CXXFLAGS% /c src/type.cpp /Fobuild/obj/type.obj 2>&1 | findstr /C:"error" && (
    echo Error: Failed to compile type.cpp
    exit /b 1
) || echo     [OK] type.obj

echo   - Compiling main.cpp...
cl.exe %CXXFLAGS% /c src/main.cpp /Fobuild/obj/main.obj 2>&1 | findstr /C:"error" && (
    echo Error: Failed to compile main.cpp
    exit /b 1
) || echo     [OK] main.obj

echo   - Compiling test_runner.cpp...
cl.exe %CXXFLAGS% /c src/test_runner.cpp /Fobuild/obj/test_runner.obj 2>&1 | findstr /C:"error" && (
    echo Error: Failed to compile test_runner.cpp
    exit /b 1
) || echo     [OK] test_runner.obj

REM Link object files
echo.
echo [*] Linking executables...

echo   - Linking nanodb.exe...
link.exe build/obj/pager.obj build/obj/type.obj build/obj/main.obj build/obj/test_runner.obj ^
    /OUT:bin/nanodb.exe /SUBSYSTEM:CONSOLE 2>&1 | findstr /C:"error" && (
    echo Error: Failed to link
    exit /b 1
) || echo     [OK] nanodb.exe

REM Create datasets directory
if not exist datasets mkdir datasets

echo.
echo ====================================================================
echo  Build Complete!
echo ====================================================================
echo.
echo Executable created: bin\nanodb.exe
echo.
echo Next steps:
echo   1. Place TPC-H data files in datasets\ directory:
echo      - customer.tbl
echo      - orders.tbl
echo      - lineitem.tbl
echo.
echo   2. Run the smoke test:
echo      bin\nanodb.exe
echo.
echo   3. Run with queries:
echo      bin\nanodb.exe queries.txt
echo.

pause
