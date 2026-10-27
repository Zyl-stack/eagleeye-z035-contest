@echo off
setlocal
cd /d "%~dp0"

if "%~1"=="" (
  echo Usage: run_player_attention_v3_ab_msvc.bat ^<real_match_b_target_stream.txt^>
  exit /b 2
)

where cl >nul 2>nul
if errorlevel 1 (
  if exist "%ProgramFiles%\Microsoft Visual Studio\18\Professional\Common7\Tools\VsDevCmd.bat" (
    call "%ProgramFiles%\Microsoft Visual Studio\18\Professional\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul
  )
)
where cl >nul 2>nul
if errorlevel 1 (
  echo Visual Studio C compiler was not found.
  exit /b 1
)

if not exist tools\build mkdir tools\build
cl /nologo /std:c11 /W4 /Iinclude ^
  tools\player_attention_v3_ab.c ^
  src\a_fovea_adapter.c ^
  src\attention.c ^
  src\attention_scheduler.c ^
  src\ball_challenger_gate.c ^
  src\bc_adapter.c ^
  src\c_config.c ^
  src\director_state.c ^
  src\eagleeye_types.c ^
  src\enhance_policy.c ^
  src\feedback_attention.c ^
  src\fovea_transaction.c ^
  src\motion_predictor.c ^
  src\player_attention_features.c ^
  src\possession_estimator.c ^
  src\roi_motion_controller.c ^
  src\roi_smoother.c ^
  /Fo:tools\build\ /Fe:tools\build\player_attention_v3_ab.exe
if errorlevel 1 exit /b 1

tools\build\player_attention_v3_ab.exe "%~1"
