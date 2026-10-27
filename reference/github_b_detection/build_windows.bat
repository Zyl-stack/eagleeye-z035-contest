@echo off
setlocal

if not exist build mkdir build

gcc -std=c99 -Wall -Wextra -Iinclude src\*.c -o build\eagleeye_c_sim.exe
if errorlevel 1 (
  echo Build failed. Install MinGW GCC, MSYS2 GCC, or use the Vitis toolchain.
  exit /b 1
)

build\eagleeye_c_sim.exe
