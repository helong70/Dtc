@echo off
rem Build script for DTC module using Arm GNU Toolchain
rem Toolchain path (adjust if needed):
set "TOOLCHAIN=D:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\11.2 2022.02\bin"
set "GCC=%TOOLCHAIN%\arm-none-eabi-gcc.exe"

echo Using toolchain: %GCC%
if not exist "%GCC%" (
  echo ERROR: arm-none-eabi-gcc not found at "%GCC%"
  echo Please check the TOOLCHAIN path in this script.
  exit /b 1
)

rem Create build output directory
if not exist build mkdir build

echo Compiling Dtc.c...
"%GCC%" -c -Wall -Wextra -O0 -g -std=c11 -I. -o build\Dtc.o Dtc.c
if errorlevel 1 (
  echo Failed to compile Dtc.c
  exit /b %errorlevel%
)

echo Compiling Dtc_cfg.c...
"%GCC%" -c -Wall -Wextra -O0 -g -std=c11 -I. -o build\Dtc_cfg.o Dtc_cfg.c
if errorlevel 1 (
  echo Failed to compile Dtc_cfg.c
  exit /b %errorlevel%
)

echo Compilation finished. Object files are in the build\\ directory.
echo build\\Dtc.o
echo build\\Dtc_cfg.o

echo Note: This script only compiles to object files (no linking). If you want to link, provide a linker script and the required startup files.
exit /b 0
