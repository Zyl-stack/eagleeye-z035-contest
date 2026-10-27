@echo off
setlocal

if not exist build mkdir build
if not exist build\obj mkdir build\obj

cl /nologo /std:c11 /W4 /Iinclude ^
  src\a_fovea_adapter.c ^
  src\bc_adapter.c ^
  src\ball_challenger_gate.c ^
  src\c_config.c ^
  src\c_performance.c ^
  src\c_runtime_log.c ^
  src\decision_snapshot.c ^
  src\director_state.c ^
  src\enhance_policy.c ^
  src\fovea_feedback.c ^
  src\feedback_attention.c ^
  src\fovea_stats.c ^
  src\fovea_transaction.c ^
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
  src\roi_motion_controller.c ^
  src\roi_smoother.c ^
  src\state_machine.c ^
  src\transaction.c ^
  /Fo:build\obj\ /Fe:build\eagleeye_c_sim.exe
if errorlevel 1 (
  echo MSVC build failed.
  exit /b 1
)

build\eagleeye_c_sim.exe

cl /nologo /std:c11 /W4 /Iinclude ^
  tools\bc_stream_demo.c ^
  src\bc_adapter.c ^
  src\ball_challenger_gate.c ^
  src\c_config.c ^
  src\c_performance.c ^
  src\c_runtime_log.c ^
  src\decision_snapshot.c ^
  src\director_state.c ^
  src\enhance_policy.c ^
  src\fovea_feedback.c ^
  src\feedback_attention.c ^
  src\fovea_stats.c ^
  src\fovea_transaction.c ^
  src\attention.c ^
  src\attention_scheduler.c ^
  src\motion_predictor.c ^
  src\possession_estimator.c ^
  src\roi_motion_controller.c ^
  src\roi_smoother.c ^
  src\eagleeye_types.c ^
  /Fo:build\obj\ /Fe:build\bc_stream_demo.exe
if errorlevel 1 (
  echo B to C stream demo build failed.
  exit /b 1
)
