@echo off
setlocal

if not exist build mkdir build
if not exist build\obj mkdir build\obj

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_scenarios.c ^
  src\attention.c ^
  src\attention_scheduler.c ^
  src\eagleeye_types.c ^
  src\state_machine.c ^
  src\transaction.c ^
  src\risk.c ^
  src\fake_data.c ^
  src\motion_predictor.c ^
  src\possession_estimator.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_scenarios.exe

if errorlevel 1 (
  echo Scenario test build failed.
  exit /b 1
)

build\test_scenarios.exe
if errorlevel 1 (
  echo Scenario tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_attention_score.c ^
  src\attention_score.c ^
  src\eagleeye_types.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_attention_score.exe

if errorlevel 1 (
  echo Attention score test build failed.
  exit /b 1
)

build\test_attention_score.exe
if errorlevel 1 (
  echo Attention score tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_possession.c ^
  src\possession_estimator.c ^
  src\eagleeye_types.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_possession.exe
if errorlevel 1 (
  echo Possession tests build failed.
  exit /b 1
)
build\test_possession.exe
if errorlevel 1 (
  echo Possession tests failed.
  exit /b 1
)

echo All tests passed.
