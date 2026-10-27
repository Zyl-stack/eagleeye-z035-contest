@echo off
setlocal
cd /d "%~dp0"

set "PYTHON_CMD="
where python >nul 2>nul
if not errorlevel 1 (
  python -c "import tkinter" >nul 2>nul
  if not errorlevel 1 set "PYTHON_CMD=python"
)
if not defined PYTHON_CMD (
  where py >nul 2>nul
  if not errorlevel 1 (
    py -3 -c "import tkinter" >nul 2>nul
    if not errorlevel 1 set "PYTHON_CMD=py -3"
  )
)
if not defined PYTHON_CMD (
  echo Python 3 with Tkinter was not found.
  echo Install Python 3 from python.org, then run this file again.
  pause
  exit /b 1
)

if not exist tools\build\visual_demo_bridge.exe (
  call build_visualizer_bridge.bat
  if errorlevel 1 (
    pause
    exit /b 1
  )
)

%PYTHON_CMD% tools\soccer_visualizer.py
if errorlevel 1 pause
