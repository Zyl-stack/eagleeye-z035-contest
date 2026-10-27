"""Run the C Mini Match bridge and render or trace its C-produced decisions."""

import argparse
import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BRIDGE = ROOT / "tools" / "build" / "visual_demo_bridge.exe"
KEY_FRAMES = (104, 105, 131, 211, 226)


def load_c_frames(player_v3=False):
    if not BRIDGE.exists():
        raise FileNotFoundError(
            "C bridge is missing. Run build_visualizer_bridge.bat from the repository root."
        )

    command = [str(BRIDGE), "6"]
    if player_v3:
        command.append("--player-v3")
    result = subprocess.run(command, check=True, capture_output=True, text=True)
    frames = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
    if len(frames) != 280:
        raise RuntimeError("C bridge returned %d frames; expected 280." % len(frames))
    return frames


def selected_target(frame):
    return next(
        (target for target in frame["targets"]
         if target["target_id"] == frame["selected"] and target.get("valid", 1)),
        None,
    )


def verify_c_payload(frames):
    required = {
        "frame_id", "selected", "fovea_view_target_id", "request_id",
        "roi_x", "roi_y", "roi_w", "roi_h",
        "desired_roi_x", "desired_roi_y", "desired_roi_w", "desired_roi_h",
        "switch_reason", "director_state", "director_event",
        "roi_motion_mode", "fast_jump_remaining", "roi_motion_step_x", "roi_motion_step_y",
        "feedback_status", "feedback_request_id", "feedback_target_id",
        "feedback_quality_flags", "feedback_age_frames",
        "base_attention", "feedback_adjustment", "final_attention",
        "feedback_attention_reason",
        "enhance_mode_name", "enhance_reason", "targets",
    }
    for frame in frames:
        missing = required.difference(frame)
        if missing:
            raise RuntimeError("C bridge frame %s is missing: %s" %
                               (frame.get("frame_id", "?"), sorted(missing)))
        if frame["selected"] != frame["fovea_view_target_id"]:
            raise RuntimeError("C selected/Fovea mismatch at frame %s" % frame["frame_id"])
        selected = selected_target(frame)
        if frame["selected"] != 65535 and selected is None:
            raise RuntimeError("C selected target is absent at frame %s" % frame["frame_id"])
        if selected is not None and "attention" not in selected:
            raise RuntimeError("C attention breakdown is absent at frame %s" % frame["frame_id"])


def event_line(frame):
    selected = selected_target(frame)
    attention = selected["attention"] if selected else 0
    return (
        "[Frame {frame}]\n"
        "Director: {director}\n"
        "Event: {event}\n"
        "ROI Motion: {roi_motion} ({fast_left} frames left)\n"
        "Feedback: {feedback} request={feedback_request} target=#{feedback_target} quality=0x{quality:08X}\n"
        "Selected: #{selected}\n"
        "Mode: {mode}\n"
        "ROI: ({x},{y},{w},{h})\n"
        "Attention (C): base={base_attention} feedback={feedback_adjustment:+d} "
        "final={final_attention} ({feedback_reason})\n"
    ).format(
        frame=frame["frame_id"], director=frame["director_state"],
        event=frame["director_event"], selected=frame["selected"],
        roi_motion=frame["roi_motion_mode"], fast_left=frame["fast_jump_remaining"],
        feedback=frame["feedback_status"], feedback_request=frame["feedback_request_id"],
        feedback_target=frame["feedback_target_id"], quality=frame["feedback_quality_flags"],
        mode=frame["enhance_mode_name"], x=frame["roi_x"], y=frame["roi_y"],
        w=frame["roi_w"], h=frame["roi_h"], attention=attention,
        base_attention=frame["base_attention"],
        feedback_adjustment=frame["feedback_adjustment"],
        final_attention=frame["final_attention"],
        feedback_reason=frame["feedback_attention_reason"],
    )


def key_event_lines(frames, start_frame):
    lines = []
    for frame in frames:
        event = frame["director_event"]
        if frame["frame_id"] >= start_frame and (
                frame["frame_id"] in KEY_FRAMES or event != "NONE"):
            lines.append(event_line(frame))
    return lines


def write_headless_trace(frames, start_frame, trace_path):
    trace_path.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "EagleEye %s PC Demo Trace" % frames[0].get("attention_mode", "SOCCER_V2"),
        "Source: C visual_demo_bridge.exe JSONL output",
        "PC VISUALIZATION VERIFIED",
        "NOT HARDWARE VERIFIED",
        "",
    ]
    lines.extend(key_event_lines(frames, start_frame))
    final = frames[-1]
    lines.append(
        "Summary: frames={frames} switches={switches} passes={passes} "
        "ball_lost={lost} bypass={bypass} bilinear={bilinear} lanczos={lanczos}".format(
            frames=final["stats_frames"], switches=final["stats_switches"],
            passes=final["stats_passes"], lost=final["stats_ball_lost"],
            bypass=final["stats_bypass"], bilinear=final["stats_bilinear"],
            lanczos=final["stats_lanczos"],
        )
    )
    trace_path.write_text("\n".join(lines), encoding="utf-8")
    return trace_path


def parse_args():
    parser = argparse.ArgumentParser(
        description="Drive the C Mini Match bridge and visualize its output."
    )
    playback = parser.add_mutually_exclusive_group()
    playback.add_argument("--realtime", action="store_true",
                          help="play the GUI at 25 FPS")
    playback.add_argument("--fast", action="store_true",
                          help="play the GUI quickly")
    parser.add_argument("--start-frame", type=int, default=1,
                        help="first C frame shown or written to the trace")
    parser.add_argument("--headless", action="store_true",
                        help="write a C-result trace instead of opening Tkinter")
    parser.add_argument("--player-v3", action="store_true",
                        help="enable the C PLAYER_DRIVEN_V3 A/B visualization mode")
    parser.add_argument("--trace", type=Path,
                        default=ROOT / "reports" / "competition_demo_trace.txt",
                        help="headless trace destination")
    return parser.parse_args()


def main():
    args = parse_args()
    if args.start_frame < 1 or args.start_frame > 280:
        print("--start-frame must be between 1 and 280.", file=sys.stderr)
        return 2
    try:
        frames = load_c_frames(args.player_v3)
        verify_c_payload(frames)
    except (FileNotFoundError, subprocess.CalledProcessError, json.JSONDecodeError,
            OSError, RuntimeError) as error:
        print("Competition demo failed: %s" % error, file=sys.stderr)
        return 1

    for line in key_event_lines(frames, args.start_frame):
        print(line, end="")

    if args.headless:
        trace_path = write_headless_trace(frames, args.start_frame, args.trace)
        print("Headless trace: %s" % trace_path)
        print("PC VISUALIZATION VERIFIED")
        print("NOT HARDWARE VERIFIED")
        return 0

    try:
        from soccer_visualizer import launch_frames
        playback = "fast" if args.fast else "realtime"
        launch_frames(frames, start_frame=args.start_frame, playback=playback)
    except Exception as error:  # GUI availability differs across PCs.
        trace_path = write_headless_trace(frames, args.start_frame, args.trace)
        print("GUI unavailable: %s" % error, file=sys.stderr)
        print("Headless trace: %s" % trace_path, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
