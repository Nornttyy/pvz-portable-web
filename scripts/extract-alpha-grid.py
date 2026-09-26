#!/usr/bin/env python3
"""Export real transparent sprite-atlas cells as trimmed production PNGs.

The generator delivers each rig or VFX sheet as a transparent grid.  This
tool only crops those existing pixels; it never paints substitute parts.
Keeping it in the repository makes every generated runtime file traceable
back to its atlas and prevents a later packaging step from treating a full
concept sheet as a sprite.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image


def parsed_names(value: str) -> list[str]:
    names = [name.strip() for name in value.split(",") if name.strip()]
    if not names:
        raise argparse.ArgumentTypeError("at least one output name is required")
    for name in names:
        if any(ch not in "abcdefghijklmnopqrstuvwxyz0123456789-" for ch in name):
            raise argparse.ArgumentTypeError(f"unsafe asset name: {name}")
    return names


def alpha_bbox(cell: Image.Image) -> tuple[int, int, int, int] | None:
    alpha = cell.getchannel("A")
    # Generator edges can contain a one-pixel, near-transparent halo.  Keep
    # visible antialiasing while ignoring fully empty grid space.
    return alpha.point(lambda value: 255 if value > 2 else 0).getbbox()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    parser.add_argument("--rows", type=int, required=True)
    parser.add_argument("--cols", type=int, required=True)
    parser.add_argument("--prefix", required=True)
    parser.add_argument("--names", type=parsed_names, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--margin", type=int, default=3)
    parser.add_argument("--role", default="vfx")
    args = parser.parse_args()

    if args.rows <= 0 or args.cols <= 0 or len(args.names) != args.rows * args.cols:
        raise SystemExit("names must contain exactly rows × cols entries")
    if args.margin < 0:
        raise SystemExit("margin cannot be negative")

    image = Image.open(args.source).convert("RGBA")
    width, height = image.size
    args.destination.mkdir(parents=True, exist_ok=True)
    entries: list[dict[str, object]] = []
    for index, name in enumerate(args.names):
        row, col = divmod(index, args.cols)
        left = round(col * width / args.cols)
        right = round((col + 1) * width / args.cols)
        top = round(row * height / args.rows)
        bottom = round((row + 1) * height / args.rows)
        cell = image.crop((left, top, right, bottom))
        bbox = alpha_bbox(cell)
        if bbox is None:
            raise SystemExit(f"{args.source}: grid cell {row + 1},{col + 1} ({name}) is empty")
        x0 = max(0, bbox[0] - args.margin)
        y0 = max(0, bbox[1] - args.margin)
        x1 = min(cell.width, bbox[2] + args.margin)
        y1 = min(cell.height, bbox[3] + args.margin)
        cropped = cell.crop((x0, y0, x1, y1))
        filename = f"{args.prefix}-{name}.png"
        output = args.destination / filename
        cropped.save(output)
        entries.append(
            {
                "file": filename,
                "width": cropped.width,
                "height": cropped.height,
                "source": str(args.source),
                "gridCell": [row + 1, col + 1],
                "role": args.role,
            }
        )

    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(json.dumps(entries, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Exported {len(entries)} real sprites from {args.source}.")


if __name__ == "__main__":
    main()
