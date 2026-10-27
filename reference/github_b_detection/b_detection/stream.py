"""Reader/writer for the line-oriented B -> C target stream V1."""

from __future__ import annotations

from collections.abc import Iterable, Iterator
from itertools import chain
from typing import TextIO

from .descriptor import BBox, FrameTargetList, TargetDescriptor, TargetType


MAGIC = "EE_B_TARGET_STREAM_V1"
STREAM_INFO = "STREAM_INFO"


def _validate_source_size(source_size: tuple[int, int]) -> tuple[int, int]:
    """Validate the original video-frame dimensions carried by STREAM_INFO."""
    width, height = source_size
    if not 0 < width <= 0xFFFF or not 0 < height <= 0xFFFF:
        raise ValueError(f"source dimensions must be in [1, 65535], got {source_size}")
    return width, height


def write_stream(
    frames: Iterable[FrameTargetList],
    output: TextIO,
    *,
    source_size: tuple[int, int] | None = None,
) -> None:
    """Write V1 frames, optionally declaring their original video resolution.

    ``source_size`` is the width/height of the input video frame, never YOLO's
    inference ``imgsz``.  Omitting it intentionally keeps old V1 files valid.
    """
    output.write(f"{MAGIC}\n")
    if source_size is not None:
        width, height = _validate_source_size(source_size)
        output.write(f"{STREAM_INFO},{width},{height}\n")
    for frame in frames:
        frame.validate()
        output.write(f"FRAME,{frame.frame_id},{frame.target_count}\n")
        for target in frame.targets:
            output.write(
                "TARGET,{frame_id},{target_id},{target_type},{x},{y},{w},{h},"
                "{confidence},{valid},{vx},{vy},{velocity_valid},{age},{miss_count}\n".format(
                    frame_id=target.frame_id,
                    target_id=target.target_id,
                    target_type=target.target_type.name,
                    x=target.bbox.x,
                    y=target.bbox.y,
                    w=target.bbox.w,
                    h=target.bbox.h,
                    confidence=target.confidence,
                    valid=int(target.valid),
                    vx=target.vx,
                    vy=target.vy,
                    velocity_valid=int(target.velocity_valid),
                    age=target.age,
                    miss_count=target.miss_count,
                )
            )
        output.write("END_FRAME\n")


def read_stream(source: TextIO) -> Iterator[FrameTargetList]:
    lines = (line.strip() for line in source)
    lines = (line for line in lines if line and not line.startswith("#"))
    try:
        header = next(lines)
    except StopIteration as exc:
        raise ValueError("empty target stream") from exc
    if header != MAGIC:
        raise ValueError(f"unsupported target stream header: {header}")

    # STREAM_INFO was added after the original V1 sample.  It is optional so
    # old V1 files still read as before; the B reader does not otherwise need
    # the resolution because descriptors already contain pixel coordinates.
    try:
        first_record = next(lines)
    except StopIteration:
        return
    fields = first_record.split(",")
    if fields[0] == STREAM_INFO:
        if len(fields) != 3:
            raise ValueError(f"invalid STREAM_INFO line: {first_record}")
        try:
            _validate_source_size((int(fields[1]), int(fields[2])))
        except ValueError as exc:
            raise ValueError(f"invalid STREAM_INFO line: {first_record}") from exc
        records = lines
    else:
        records = chain((first_record,), lines)

    for line in records:
        fields = line.split(",")
        if len(fields) != 3 or fields[0] != "FRAME":
            raise ValueError(f"expected FRAME line, got: {line}")
        frame_id = int(fields[1])
        count = int(fields[2])
        targets: list[TargetDescriptor] = []
        for _ in range(count):
            try:
                target_line = next(lines)
            except StopIteration as exc:
                raise ValueError("target stream ended inside a frame") from exc
            values = target_line.split(",")
            if len(values) != 15 or values[0] != "TARGET":
                raise ValueError(f"invalid TARGET line: {target_line}")
            target_frame_id = int(values[1])
            targets.append(
                TargetDescriptor(
                    frame_id=target_frame_id,
                    target_id=int(values[2]),
                    target_type=TargetType[values[3]],
                    bbox=BBox(*(int(value) for value in values[4:8])),
                    confidence=int(values[8]),
                    valid=bool(int(values[9])),
                    vx=int(values[10]),
                    vy=int(values[11]),
                    velocity_valid=bool(int(values[12])),
                    age=int(values[13]),
                    miss_count=int(values[14]),
                )
            )
        try:
            end_line = next(lines)
        except StopIteration as exc:
            raise ValueError("target stream is missing END_FRAME") from exc
        if end_line != "END_FRAME":
            raise ValueError(f"expected END_FRAME, got: {end_line}")
        frame = FrameTargetList(frame_id=frame_id, targets=tuple(targets))
        frame.validate()
        yield frame
