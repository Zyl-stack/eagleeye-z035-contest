"""Fine-tune YOLO11n for PLAYER and BALL detection."""

from __future__ import annotations

import argparse
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def parsed_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Train the EagleEye football detector.")
    parser.add_argument("--data", type=Path, default=ROOT / "datasets" / "football_players_ball.yaml")
    parser.add_argument("--model", default="yolo11n.pt")
    parser.add_argument("--epochs", type=int, default=100)
    parser.add_argument("--imgsz", type=int, default=1280)
    parser.add_argument("--batch", type=int, default=4)
    parser.add_argument("--device", default="auto", help="auto, cpu, or a CUDA device such as 0")
    parser.add_argument("--project", type=Path, default=ROOT / "runs" / "football_detector")
    parser.add_argument("--name", default="yolo11n_1280")
    parser.add_argument("--workers", type=int, default=2)
    parser.add_argument(
        "--fraction",
        type=float,
        default=1.0,
        help="fraction of each split for a fast smoke run; use 1.0 for the real training run",
    )
    parser.add_argument(
        "--no-val",
        action="store_true",
        help="skip validation; intended only for a short local smoke run",
    )
    parser.add_argument("--no-export", action="store_true", help="do not export best.pt to ONNX")
    return parser.parse_args()


def resolve_device(requested: str) -> str:
    if requested != "auto":
        return requested
    import torch

    return "0" if torch.cuda.is_available() else "cpu"


def main() -> int:
    args = parsed_args()
    if not args.data.is_file():
        raise FileNotFoundError(
            f"dataset YAML not found: {args.data}. Prepare PLAYER/BALL labels first."
        )
    if not 0.0 < args.fraction <= 1.0:
        raise ValueError("--fraction must be in (0, 1]")
    try:
        from ultralytics import YOLO
    except ModuleNotFoundError as exc:
        raise RuntimeError("Install requirements-vision.txt before training.") from exc

    device = resolve_device(args.device)
    if device == "cpu":
        print("WARNING: CPU training is only a smoke test; use a CUDA GPU for the 100-epoch run.")
    model = YOLO(args.model)
    model.train(
        data=str(args.data),
        epochs=args.epochs,
        imgsz=args.imgsz,
        batch=args.batch,
        device=device,
        workers=args.workers,
        project=str(args.project),
        name=args.name,
        patience=20,
        pretrained=True,
        optimizer="auto",
        cache=False,
        fraction=args.fraction,
        val=not args.no_val,
    )
    best = args.project / args.name / "weights" / "best.pt"
    if not best.is_file():
        raise RuntimeError(f"training completed but did not create {best}")
    if not args.no_export:
        YOLO(str(best)).export(format="onnx", imgsz=args.imgsz, dynamic=True)
    print(f"best checkpoint: {best}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
