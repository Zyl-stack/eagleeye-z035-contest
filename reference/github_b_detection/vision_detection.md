# Real Football Detection (B Side)

## Selected model

The first real-image detector is **YOLO11n Detect**, initialized from
`yolo11n.pt` and fine-tuned for two classes: `PLAYER` and `BALL`. It is small
enough for a PC demo and later ONNX export, while the `1280` input size gives a
small football substantially more pixels than a 640-pixel input. This is a
detector, not a replacement for B's stable-ID tracker.

The unmodified COCO checkpoint is only a baseline: it maps COCO `person` to
`PLAYER` and COCO `sports ball` to `BALL`. A football-specific checkpoint must
be evaluated before claiming real football accuracy.

## Dataset contract

Use the standard YOLO layout below. Do not commit videos, images, labels,
weights, or `runs/` output to Git.

```text
datasets/football_yolo/
  images/train/   labels/train/
  images/val/     labels/val/
  images/test/    labels/test/
```

Each label line is `class center_x center_y width height` with normalized
coordinates. Class `0` is `PLAYER`; class `1` is `BALL`.

## Roboflow data preparation

The Stage-1 sources are the following public Roboflow Universe datasets:

- [Football Players Detection v1](https://universe.roboflow.com/roboflow-jvuqo/football-players-detection-3zvbc/dataset/1), CC BY 4.0.
- [Football Ball Detection v4](https://universe.roboflow.com/roboflow-jvuqo/football-ball-detection-rejhg/dataset/4), CC BY 4.0.

In a browser, choose **Download Dataset** then **YOLOv11** for each one.
Extract the downloads in the ignored folders shown below; retain the Roboflow
licence/attribution information with the project report.

```text
datasets/raw/football_players/  # player export
datasets/raw/football_ball/     # ball export
```

Merge them without requiring an API key. Every annotation from the player
source's original `player` class is rewritten to class `0`; every annotation
from the ball source is rewritten to class `1`. The Player source also contains
ball, goalkeeper and referee annotations: these are deliberately discarded so
they cannot be silently mislabeled as PLAYER. This intentionally excludes
goalkeeper/referee from the first-stage supervised class list.

```powershell
py -3 tools/prepare_roboflow_two_class_dataset.py `
  --players-root datasets/raw/football_players `
  --balls-root datasets/raw/football_ball
```

The tool writes `datasets/football_yolo/dataset_manifest.json` with image and
box totals. Then run the audit below before training. It rejects missing label
pairs, invalid YOLO fields, non-PLAYER/BALL classes, and a training split that
does not contain both classes.

```powershell
py -3 tools/audit_football_dataset.py
```

The combined validation/test splits are source-mixed, so report results per
class and include failure clips from the actual broadcast-style Demo video.

SoccerNet Tracking is a suitable source for football clips and tracklets, but
its MOT CSV class field is not a PLAYER/BALL class label. Convert or annotate
the data explicitly before using it for this two-class detector; do not infer a
ball label merely from a small bounding box.

## Training

```powershell
py -3 -m pip install -r requirements-vision.txt
py -3 tools/train_football_detector.py --device 0 --epochs 100 --imgsz 1280 --batch 8
```

The current PC has CPU-only PyTorch, so full training there would be very slow.
Use `--device cpu --epochs 1 --batch 1` only to smoke-test a prepared dataset,
then run the 100-epoch job on a CUDA-capable workstation or cloud GPU. The
training script exports `best.pt` to ONNX unless `--no-export` is set.

For this PC, use a small fraction only to verify the pipeline and weight-file
output; it is not a quality result:

```powershell
py -3 tools/train_football_detector.py `
  --device cpu --epochs 1 --imgsz 640 --batch 1 --workers 0 `
  --fraction 0.01 --name smoke_cpu --no-val --no-export
```

After training, create the reportable Precision, Recall, mAP50 and mAP50-95
summary from Ultralytics' `results.csv`:

```powershell
py -3 tools/summarize_yolo_metrics.py `
  --run-dir runs/football_detector/yolo11n_1280
```

## Video to B->C stream

```powershell
py -3 tools/detect_video_to_b_stream.py `
  --source path/to/match.mp4 `
  --model runs/football_detector/yolo11n_1280/weights/best.pt `
  --output build/real_match_b_target_stream.txt `
  --preview-output build/real_match_annotated.mp4
```

For a high-resolution wide shot where the ball becomes too small after full-frame
resize, add `--tile-size 960`. B runs overlapping edge-aligned source-image
tiles, removes duplicate boxes, and maps every resulting box back to the
original video coordinates before writing the stream. This improves small-ball
visibility but is slower; it does not change `STREAM_INFO` or the B→C contract.

The command runs `YOLO -> CandidateDetection -> MultiObjectTracker ->
EE_B_TARGET_STREAM_V1`. Feed the resulting stream to C's `bc_adapter`; B does
not choose possession, attention, or Fovea ROI. It also writes a sibling
`*.metrics.json` file containing the number of frames processed, measured
detector-only FPS. Keep short successful and failed annotated clips (small
ball, blur, occlusion, crowd) with the experiment report.

The B->C interface accepts at most 16 candidates per frame. The detector asks
YOLO for a larger set first, then keeps BALL candidates ahead of PLAYER
candidates before applying this cap. This protects a low-confidence small ball
from being squeezed out by a wide-shot player crowd.

## Qualitative cases and regression clip

Use `tools/detect_image_to_b_report.py` to create an annotated image and a
machine-readable detection report for a success or failure case. For a
deterministic B->C regression test, `tools/create_static_regression_clip.py`
can turn one labelled football frame into a repeated-frame clip. It verifies
that the real model, tracker, stable ID and stream writer agree, but it must
not be presented as a motion, occlusion, or broadcast-video result.
