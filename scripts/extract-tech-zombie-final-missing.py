#!/usr/bin/env python3
"""Apply the final generated, genuinely separate zombie rig pieces.

This is intentionally an asset-only post-pass for the generated tech-zombie
atlases.  It replaces the three former forearm-and-hand compatibility pieces
with standalone hand sprites, adds wrench-tech's third armour damage state,
and writes ten distinct damaged outer-upper arm pieces.  No game source or old
asset is read or modified.
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

ARM_ATLAS = "zombie-outer-upper-damaged-rig-atlas-v1.png"
HAND_SOURCES = {
    "jammer-aerial": "jammer-aerial-hand-rig-v2.png",
    "magnet-salvager": "magnet-salvager-hand-rig-v2.png",
    "holo-decoy": "holo-decoy-hand-rig-v2.png",
}
BUCKET_ATLAS = "mechanical-hands-bucket-rig-atlas-v1.png"

# Verified alpha-connected components in row-major order from ARM_ATLAS.
ARM_BOUNDS: dict[str, tuple[int, int, int, int]] = {
    "leak-pack": (56, 6, 371, 380),
    "weld-shield": (462, 14, 742, 379),
    "turbine-boot": (841, 6, 1099, 376),
    "cable-hook": (1225, 6, 1533, 389),
    "wrench-tech": (1698, 10, 1940, 391),
    "bolt-thrower": (48, 395, 333, 780),
    "hover-drone": (457, 395, 722, 770),
    "jammer-aerial": (828, 401, 1110, 782),
    "magnet-salvager": (1278, 401, 1538, 774),
    "holo-decoy": (1665, 395, 1941, 781),
}

# The lower-right component is a third stage bucket-armour prop.  The source
# rectangle intentionally excludes the bottom-left hologram-hand component.
BUCKET_BOUND = (682, 677, 1197, 1159)

# Keep visual scale compatible with each existing normal outer-upper rig part.
ARM_TARGETS = {
    "leak-pack": (179, 256),
    "weld-shield": (174, 242),
    "turbine-boot": (124, 245),
    "cable-hook": (159, 218),
    "wrench-tech": (169, 274),
    "bolt-thrower": (192, 277),
    "hover-drone": (138, 219),
    "jammer-aerial": (180, 224),
    "magnet-salvager": (177, 272),
    "holo-decoy": (144, 235),
}

# These are palms only (no forearm), so their target maxima are intentionally
# shorter than the old temporary forearm-and-hand compatibility pieces.
HAND_MAX = {
    "jammer-aerial": (174, 174),
    "magnet-salvager": (168, 168),
    "holo-decoy": (164, 164),
}


def trim(image: Image.Image, bbox: tuple[int, int, int, int] | None = None) -> Image.Image:
    source = image.crop(bbox) if bbox else image
    alpha_box = source.getchannel("A").getbbox()
    if alpha_box is None:
        raise ValueError(f"empty alpha crop {bbox}")
    l, t, r, b = alpha_box
    return source.crop((max(0, l - PAD), max(0, t - PAD), min(source.width, r + PAD), min(source.height, b + PAD)))


def resize_exact(image: Image.Image, size: tuple[int, int]) -> Image.Image:
    return image.resize(size, Image.Resampling.LANCZOS)


def resize_inside(image: Image.Image, maximum: tuple[int, int]) -> Image.Image:
    width, height = image.size
    scale = min(maximum[0] / width, maximum[1] / height)
    return image.resize((max(1, round(width * scale)), max(1, round(height * scale))), Image.Resampling.LANCZOS)


def source_ref(file_name: str, bbox: tuple[int, int, int, int] | None = None) -> str:
    suffix = "full-alpha-component" if bbox is None else ",".join(map(str, bbox))
    return f"generated/{file_name}#{suffix}"


def record(file_name: str, image: Image.Image, source: str, role: str, slug: str) -> dict[str, Any]:
    output = PARTS / file_name
    image.save(output, "PNG")
    return {
        "file": f"parts/{file_name}",
        "width": image.width,
        "height": image.height,
        "source": source,
        "role": role,
        "slug": slug,
    }


def replace_row(rows: list[dict[str, Any]], row: dict[str, Any]) -> None:
    rows[:] = [item for item in rows if item["file"] != row["file"]]
    rows.append(row)


def main() -> None:
    PARTS.mkdir(parents=True, exist_ok=True)
    document = json.loads(MANIFEST.read_text())
    rows: list[dict[str, Any]] = document["parts"]

    arm_source = Image.open(GENERATED / ARM_ATLAS).convert("RGBA")
    for slug, bbox in ARM_BOUNDS.items():
        output = resize_exact(trim(arm_source, bbox), ARM_TARGETS[slug])
        replace_row(
            rows,
            record(
                f"{slug}-outer-upper-damaged.png",
                output,
                source_ref(ARM_ATLAS, bbox),
                "independent damaged outer upper arm; torn/broken state",
                slug,
            ),
        )

    for slug, source_name in HAND_SOURCES.items():
        source = Image.open(GENERATED / source_name).convert("RGBA")
        output = resize_inside(trim(source), HAND_MAX[slug])
        replace_row(
            rows,
            record(
                f"{slug}-hand.png",
                output,
                source_ref(source_name),
                "standalone palm and fingers only; no forearm",
                slug,
            ),
        )

    bucket_source = Image.open(GENERATED / BUCKET_ATLAS).convert("RGBA")
    bucket = resize_inside(trim(bucket_source, BUCKET_BOUND), (222, 228))
    replace_row(
        rows,
        record(
            "wrench-tech-prop-damage2.png",
            bucket,
            source_ref(BUCKET_ATLAS, BUCKET_BOUND),
            "bucket armour prop damage stage 2; severe hole, torn edge, split chain",
            "wrench-tech",
        ),
    )

    # Final assets cover these former gaps.  Keep the truly unavailable hat
    # parts and turbine pack honest rather than adding compatibility fakes.
    document["missing"] = [
        item
        for item in document.get("missing", [])
        if item.get("runtime") not in {"outer-upper-damaged", "prop-damage2"}
    ]
    document["limitations"] = [
        item for item in document.get("limitations", []) if item.get("part") != "hand"
    ]
    document["parts"] = rows
    MANIFEST.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n")
    print("updated 10 damaged upper arms, 3 standalone hands, and wrench prop-damage2")


if __name__ == "__main__":
    main()
