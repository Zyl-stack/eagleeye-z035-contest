@echo off
setlocal

if not exist build mkdir build
if not exist build\obj mkdir build\obj

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_medium_risk_boundaries.c ^
  src\attention.c ^
  src\attention_scheduler.c ^
  src\feedback_attention.c ^
  src\fovea_feedback.c ^
  src\motion_predictor.c ^
  src\roi_motion_controller.c ^
  src\roi_smoother.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_medium_risk_boundaries.exe
if errorlevel 1 (
  echo Medium-risk boundary test build failed.
  exit /b 1
)

build\test_medium_risk_boundaries.exe %*
if errorlevel 1 (
  echo Medium-risk boundary tests failed.
  exit /b 1
)

echo All medium-risk boundary tests passed.
