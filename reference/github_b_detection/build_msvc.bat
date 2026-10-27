@echo off
setlocal

if not exist build mkdir build
if not exist build\obj mkdir build\obj

cl /nologo /std:c11 /W4 /Iinclude ^
  src\attention.c ^
  src\attention_scheduler.c ^
  src\attention_score.c ^
  src\eagleeye_types.c ^
  src\fake_data.c ^
  src\main.c ^
  src\motion_predictor.c ^
  src\possession_estimator.c ^
  src\platform_io_sim.c ^
  src\risk.c ^
  src\state_machine.c ^
  src\transaction.c ^
  /Fo:build\obj\ /Fe:build\eagleeye_c_sim.exe
if errorlevel 1 (
  echo MSVC build failed.
  exit /b 1
)

build\eagleeye_c_sim.exe
