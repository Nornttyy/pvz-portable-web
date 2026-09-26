#!/usr/bin/env python3
"""Cut the five dedicated generated tech-zombie leg atlases into rig pieces.

The source art is a fixed 3 x 2 sheet per character.  This pass only crops
and removes the generator's very-low-alpha backdrop haze; it never redraws,
recolours, composites, or substitutes any older zombie asset.
"""

from __future__ import annotations

import json
from pathlib import Path
from typing import Final

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
TECH = ROOT / "art" / "tech"
GENERATED = TECH / "generated"
PARTS = TECH / "parts"
MANIFEST = TECH / "leg-parts.json"

SLUGS: Final = (
    "leak-pack",
    "weld-shield",
    "turbine-boot",
    "cable-hook",
    "wrench-tech",
)

# Atlas order is intentionally fixed by the image-generation prompt.
# (file suffix, column, row, original Zombie.reanim native canvas, role)
LAYOUT: Final = (
    ("inner-leg-upper", 0, 0, (15, 26), "inner upper leg / thigh only"),
    ("inner-leg-lower", 1, 0, (32, 36), "inner lower leg / shin only"),
    ("inner-leg-foot", 2, 0, (27, 17), "inner foot / boot only"),
    ("outer-leg-upper", 0, 1, (21, 39), "outer upper leg / thigh only"),
    ("outer-leg-lower", 1, 1, (24, 30), "outer lower leg / shin only"),
    ("outer-leg-foot", 2, 1, (42, 21), "outer foot / boot only"),
)

CELL_W: Final = 512
CELL_H: Final = 512
ALPHA_CUTOFF: Final = 12
PADDING: Final = 3


def clear_backdrop_and_trim(source: Image.Image) -> Image.Image:
    """Preserve real art while clearing only the generator's faint matte haze."""
    rgba = source.convert("RGBA")
    pixels = rgba.load()
    for y in range(rgba.height):
        for x in range(rgba.width):
            red, green, blue, alpha = pixels[x, y]
            if alpha < ALPHA_CUTOFF:
                pixels[x, y] = (0, 0, 0, 0)

    alpha = rgba.getchannel("A")
    bounds = alpha.getbbox()
    if bounds is None:
        raise ValueError("grid cell has no usable alpha after backdrop cleanup")
    left, top, right, bottom = bounds
    left = max(0, left - PADDING)
    top = max(0, top - PADDING)
    right = min(rgba.width, right + PADDING)
    bottom = min(rgba.height, bottom + PADDING)
    return rgba.crop((left, top, right, bottom))


def crop_cell(atlas: Image.Image, column: int, row: int) -> Image.Image:
    left = column * CELL_W
    top = row * CELL_H
    return clear_backdrop_and_trim(atlas.crop((left, top, left + CELL_W, top + CELL_H)))


def validate(image: Image.Image, output: Path) -> None:
    alpha = image.getchannel("A")
    minimum, maximum = alpha.getextrema()
    if image.mode != "RGBA" or alpha.getbbox() is None or minimum != 0 or maximum == 0:
        raise ValueError(f"invalid transparent rig part: {output}")


def main() -> None:
    PARTS.mkdir(parents=True, exist_ok=True)
    records: list[dict[str, object]] = []
    for slug in SLUGS:
        atlas_name = f"{slug}-leg-rig-atlas-v1.png"
        atlas_path = GENERATED / atlas_name
        atlas = Image.open(atlas_path).convert("RGBA")
        if atlas.size != (CELL_W * 3, CELL_H * 2):
            raise ValueError(f"unexpected atlas size for {atlas_path}: {atlas.size}")

        for suffix, column, row, runtime_size, role in LAYOUT:
            filename = f"{slug}-{suffix}.png"
            output = PARTS / filename
            # These are new filenames.  Refuse an accidental overwrite of an
            # independently produced rig asset on a subsequent run.
            if output.exists():
                raise FileExistsError(output)
            part = crop_cell(atlas, column, row)
            validate(part, output)
            part.save(output, format="PNG", optimize=True)
            records.append({
                "file": f"parts/{filename}",
                "width": part.width,
                "height": part.height,
                "source": f"generated/{atlas_name}#grid({column},{row})",
                "role": role,
                "slug": slug,
                "runtime_canvas": {"width": runtime_size[0], "height": runtime_size[1]},
            })

    document = {
        "schema": 1,
        "description": (
            "Independent RGBA technology-zombie leg rig parts cut only from "
            "the five dedicated generated 3x2 leg atlases."
        ),
        "parts": records,
    }
    MANIFEST.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"exported {len(records)} independent leg parts")
    for item in records:
        print(f"{item['file']}: {item['width']}x{item['height']}")


if __name__ == "__main__":
    main()
