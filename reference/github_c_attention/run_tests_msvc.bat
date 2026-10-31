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

if not exist build mkdir build
if not exist build\obj mkdir build\obj

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_scenarios.c ^
  src\attention.c ^
  src\feedback_attention.c ^
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

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_a_fovea_adapter.c ^
  src\a_fovea_adapter.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_a_fovea_adapter.exe
if errorlevel 1 (
  echo A Fovea adapter test build failed.
  exit /b 1
)
build\test_a_fovea_adapter.exe
if errorlevel 1 (
  echo A Fovea adapter tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_fovea_transaction.c ^
  src\fovea_transaction.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_fovea_transaction.exe
if errorlevel 1 (
  echo Fovea transaction test build failed.
  exit /b 1
)
build\test_fovea_transaction.exe
if errorlevel 1 (
  echo Fovea transaction tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_bc_adapter.c ^
  src\bc_adapter.c ^
  src\attention.c ^
  src\feedback_attention.c ^
  src\attention_scheduler.c ^
  src\possession_estimator.c ^
  src\eagleeye_types.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_bc_adapter.exe
if errorlevel 1 (
  echo B to C adapter test build failed.
  exit /b 1
)
build\test_bc_adapter.exe
if errorlevel 1 (
  echo B to C adapter tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_ball_challenger_gate.c ^
  src\attention.c ^
  src\feedback_attention.c ^
  src\attention_scheduler.c ^
  src\ball_challenger_gate.c ^
  src\c_config.c ^
  src\player_attention_features.c ^
  src\eagleeye_types.c ^
  src\motion_predictor.c ^
  src\roi_motion_controller.c ^
  src\roi_smoother.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_ball_challenger_gate.exe
if errorlevel 1 (
  echo BALL challenger gate test build failed.
  exit /b 1
)
build\test_ball_challenger_gate.exe
if errorlevel 1 (
  echo BALL challenger gate tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_player_attention_v3.c ^
  src\attention_scheduler.c ^
  src\eagleeye_types.c ^
  src\player_attention_features.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_player_attention_v3.exe
if errorlevel 1 (
  echo Player Attention V3 test build failed.
  exit /b 1
)
build\test_player_attention_v3.exe
if errorlevel 1 (
  echo Player Attention V3 tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_c_stability.c ^
  src\attention.c ^
  src\feedback_attention.c ^
  src\attention_scheduler.c ^
  src\c_config.c ^
  src\player_attention_features.c ^
  src\c_runtime_log.c ^
  src\eagleeye_types.c ^
  src\motion_predictor.c ^
  src\roi_motion_controller.c ^
  src\roi_smoother.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_c_stability.exe
if errorlevel 1 (
  echo C stability test build failed.
  exit /b 1
)
build\test_c_stability.exe
if errorlevel 1 (
  echo C stability tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_match_regression.c ^
  src\attention.c ^
  src\feedback_attention.c ^
  src\attention_scheduler.c ^
  src\c_config.c ^
  src\player_attention_features.c ^
  src\decision_snapshot.c ^
  src\director_state.c ^
  src\eagleeye_types.c ^
  src\enhance_policy.c ^
  src\fovea_feedback.c ^
  src\fovea_stats.c ^
  src\motion_predictor.c ^
  src\possession_estimator.c ^
  src\roi_motion_controller.c ^
  src\roi_smoother.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_match_regression.exe
if errorlevel 1 (
  echo Match regression test build failed.
  exit /b 1
)
build\test_match_regression.exe
if errorlevel 1 (
  echo Match regression tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_competition_features_v2.c ^
  src\attention.c ^
  src\feedback_attention.c ^
  src\attention_scheduler.c ^
  src\c_config.c ^
  src\player_attention_features.c ^
  src\decision_snapshot.c ^
  src\director_state.c ^
  src\eagleeye_types.c ^
  src\enhance_policy.c ^
  src\fovea_feedback.c ^
  src\fovea_stats.c ^
  src\motion_predictor.c ^
  src\roi_motion_controller.c ^
  src\roi_smoother.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_competition_features_v2.exe
if errorlevel 1 (
  echo Competition features V2 test build failed.
  exit /b 1
)
build\test_competition_features_v2.exe
if errorlevel 1 (
  echo Competition features V2 tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_latency_aware_prediction.c ^
  src\a_fovea_adapter.c ^
  src\attention.c ^
  src\feedback_attention.c ^
  src\attention_scheduler.c ^
  src\c_config.c ^
  src\player_attention_features.c ^
  src\eagleeye_types.c ^
  src\motion_predictor.c ^
  src\roi_motion_controller.c ^
  src\roi_smoother.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_latency_aware_prediction.exe
if errorlevel 1 (
  echo Latency-aware prediction test build failed.
  exit /b 1
)
build\test_latency_aware_prediction.exe
if errorlevel 1 (
  echo Latency-aware prediction tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_dual_mode_roi_control.c ^
  src\a_fovea_adapter.c ^
  src\motion_predictor.c ^
  src\roi_motion_controller.c ^
  src\roi_smoother.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_dual_mode_roi_control.exe
if errorlevel 1 (
  echo Dual-mode ROI controller test build failed.
  exit /b 1
)
build\test_dual_mode_roi_control.exe
if errorlevel 1 (
  echo Dual-mode ROI controller tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_fovea_feedback.c ^
  src\a_fovea_adapter.c ^
  src\attention.c ^
  src\feedback_attention.c ^
  src\attention_scheduler.c ^
  src\eagleeye_types.c ^
  src\fovea_feedback.c ^
  src\fovea_feedback_sim.c ^
  src\fovea_transaction.c ^
  src\motion_predictor.c ^
  src\roi_motion_controller.c ^
  src\roi_smoother.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_fovea_feedback.exe
if errorlevel 1 (
  echo Fovea feedback transaction test build failed.
  exit /b 1
)
build\test_fovea_feedback.exe
if errorlevel 1 (
  echo Fovea feedback transaction tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_feedback_aware_attention.c ^
  src\attention.c ^
  src\attention_scheduler.c ^
  src\bc_adapter.c ^
  src\eagleeye_types.c ^
  src\feedback_attention.c ^
  src\fovea_feedback.c ^
  src\fovea_feedback_sim.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_feedback_aware_attention.exe
if errorlevel 1 (
  echo Feedback-aware Attention test build failed.
  exit /b 1
)
build\test_feedback_aware_attention.exe
if errorlevel 1 (
  echo Feedback-aware Attention tests failed.
  exit /b 1
)

cl /nologo /std:c11 /W4 /Iinclude ^
  tests\test_velocity_boundaries.c ^
  src\attention.c ^
  src\enhance_policy.c ^
  src\feedback_attention.c ^
  /Fo:build\obj\ ^
  /Fe:build\test_velocity_boundaries.exe
if errorlevel 1 (
  echo Signed velocity boundary test build failed.
  exit /b 1
)
build\test_velocity_boundaries.exe
if errorlevel 1 (
  echo Signed velocity boundary tests failed.
  exit /b 1
)

echo All tests passed.
