#!/usr/bin/env python3
"""Cut the five generated mechanical-zombie leg rig atlases into real RGBA parts.

The source atlases are new, transparent, generated production art.  This script
only writes `art/tech/parts/` and the matching rows in `zombie-parts.json`; it
does not touch code or legacy assets.
"""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
TECH = ROOT / "art" / "tech"
GENERATED = TECH / "generated"
PARTS = TECH / "parts"
MANIFEST = TECH / "zombie-parts.json"
PAD = 2

# Each input atlas is a 3x2 grid.  These bounds were verified from the six
# alpha-connected components: top row inner upper/lower/foot; bottom row outer
# upper/lower/foot.
BOUNDS: dict[str, list[tuple[int, int, int, int]]] = {
    "bolt-thrower": [
        (132, 10, 486, 484), (650, 8, 910, 489), (1020, 103, 1405, 474),
        (99, 518, 448, 974), (632, 512, 933, 1001), (1074, 591, 1453, 966),
    ],
    "hover-drone": [
        (182, 27, 456, 453), (696, 26, 882, 472), (1139, 232, 1417, 498),
        (233, 517, 457, 943), (663, 529, 871, 945), (1118, 707, 1428, 979),
    ],
    "jammer-aerial": [
        (128, 29, 452, 498), (643, 15, 904, 505), (1044, 112, 1452, 498),
        (101, 515, 467, 981), (669, 507, 896, 986), (1048, 596, 1485, 993),
    ],
    "magnet-salvager": [
        (136, 46, 447, 494), (629, 42, 890, 485), (1039, 157, 1402, 493),
        (133, 536, 441, 970), (632, 536, 884, 973), (1030, 629, 1411, 968),
    ],
    "holo-decoy": [
        (182, 45, 471, 470), (668, 35, 909, 485), (1076, 255, 1379, 480),
        (213, 515, 477, 990), (666, 519, 892, 979), (1046, 739, 1399, 984),
    ],
}

PART_NAMES = (
    "inner-leg-upper",
    "inner-leg-lower",
    "inner-leg-foot",
    "outer-leg-upper",
    "outer-leg-lower",
    "outer-leg-foot",
)

# Compact but readable gameplay scale.  They preserve each source silhouette's
# aspect ratio, rather than stretching legs or silently baking parts together.
MAX_SIZES: dict[str, list[tuple[int, int]]] = {
    "bolt-thrower": [(130, 120), (92, 130), (175, 88), (130, 120), (94, 130), (175, 88)],
    "hover-drone": [(120, 115), (86, 125), (158, 82), (120, 115), (86, 125), (158, 82)],
    "jammer-aerial": [(142, 118), (104, 126), (180, 88), (142, 118), (104, 126), (180, 88)],
    "magnet-salvager": [(150, 122), (110, 130), (188, 92), (150, 122), (110, 130), (188, 92)],
    "holo-decoy": [(138, 125), (96, 132), (178, 86), (138, 125), (96, 132), (178, 86)],
}


def trim(image: Image.Image, bbox: tuple[int, int, int, int]) -> Image.Image:
    crop = image.crop(bbox)
    alpha_box = crop.getchannel("A").getbbox()
    if alpha_box is None:
        raise ValueError(f"empty alpha crop {bbox}")
    left, top, right, bottom = alpha_box
    return crop.crop(
        (max(0, left - PAD), max(0, top - PAD), min(crop.width, right + PAD), min(crop.height, bottom + PAD))
    )


def resize_inside(image: Image.Image, maximum: tuple[int, int]) -> Image.Image:
    scale = min(maximum[0] / image.width, maximum[1] / image.height)
    return image.resize(
        (max(1, round(image.width * scale)), max(1, round(image.height * scale))),
        Image.Resampling.LANCZOS,
    )


def replace_row(rows: list[dict[str, Any]], item: dict[str, Any]) -> None:
    rows[:] = [row for row in rows if row["file"] != item["file"]]
    rows.append(item)


def main() -> None:
    document = json.loads(MANIFEST.read_text())
    rows: list[dict[str, Any]] = document["parts"]
    PARTS.mkdir(parents=True, exist_ok=True)

    for slug, bounds in BOUNDS.items():
        atlas_name = f"{slug}-legs-rig-atlas-v1.png"
        atlas = Image.open(GENERATED / atlas_name).convert("RGBA")
        # v1 of this extractor accidentally used `inner-foot` / `outer-foot`
        # while the other five rigs and C++ runtime use the explicit
        # `*-leg-foot` spelling.  Remove only those self-generated aliases
        # before emitting the canonical production names.
        for legacy in ("inner-foot", "outer-foot"):
            legacy_file = PARTS / f"{slug}-{legacy}.png"
            if legacy_file.exists():
                legacy_file.unlink()
            rows[:] = [row for row in rows if row["file"] != f"parts/{slug}-{legacy}.png"]
        for name, bbox, maximum in zip(PART_NAMES, bounds, MAX_SIZES[slug], strict=True):
            image = resize_inside(trim(atlas, bbox), maximum)
            file_name = f"{slug}-{name}.png"
            image.save(PARTS / file_name, "PNG")
            replace_row(
                rows,
                {
                    "file": f"parts/{file_name}",
                    "width": image.width,
                    "height": image.height,
                    "source": f"generated/{atlas_name}#{','.join(map(str, bbox))}",
                    "role": f"independent {name.replace('-', ' ')} leg rig part",
                    "slug": slug,
                },
            )

    document["parts"] = rows
    MANIFEST.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n")
    print(f"wrote {len(BOUNDS) * len(PART_NAMES)} independent leg parts")


if __name__ == "__main__":
    main()
