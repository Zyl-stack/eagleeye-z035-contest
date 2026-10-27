@echo off
setlocal
cd /d "%~dp0"

where cl >nul 2>nul
if errorlevel 1 (
  if exist "%ProgramFiles%\Microsoft Visual Studio\18\Professional\Common7\Tools\VsDevCmd.bat" (
    call "%ProgramFiles%\Microsoft Visual Studio\18\Professional\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul
  )
)

where cl >nul 2>nul
if errorlevel 1 (
  echo Visual Studio C compiler was not found.
  echo Open a Visual Studio Developer Command Prompt and run this file again.
  exit /b 1
)

if not exist tools\build mkdir tools\build
cl /nologo /std:c11 /W4 /Iinclude ^
  tools\visual_demo_bridge.c ^
  src\attention.c ^
  src\attention_scheduler.c ^
  src\eagleeye_types.c ^
  src\fake_data.c ^
  src\motion_predictor.c ^
  src\possession_estimator.c ^
  /Fo:tools\build\ /Fe:tools\build\visual_demo_bridge.exe
if errorlevel 1 (
  echo Visualizer bridge build failed.
  exit /b 1
)
echo Visualizer bridge built successfully.
