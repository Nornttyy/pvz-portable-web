#!/usr/bin/env python3
"""Export right-facing Arc Orchid head and separate aperture from v2 art."""

from __future__ import annotations

import json
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "art/tech/generated/arc-orchid-rig-atlas-v2.png"
PARTS = ROOT / "art/tech/parts"
MANIFEST = ROOT / "art/tech/plant-parts.json"

SPECS = {
    "small-head": (44, 5, 444, 409),
    "small-blink": (535, 24, 925, 409),
    # A tight crop of the luminous aperture on the front-only component.
    # It is intentionally much narrower than the complete front petal, so
    # the separate mouth bone does not duplicate the head silhouette.
    "small-mouth": (1310, 128, 1425, 338),
}


def trim(image: Image.Image, padding: int = 2) -> Image.Image:
    box = image.getchannel("A").getbbox()
    if box is None:
        raise ValueError("empty alpha crop")
    part = image.crop(box)
    result = Image.new("RGBA", (part.width + padding * 2, part.height + padding * 2))
    result.alpha_composite(part, (padding, padding))
    return result


def main() -> None:
    atlas = Image.open(SOURCE).convert("RGBA")
    output: dict[str, Image.Image] = {}
    for name, box in SPECS.items():
        item = atlas.crop(box)
        output[name] = trim(item)
        output[name].save(PARTS / f"arc-orchid-{name}.png", optimize=True)

    rows = json.loads(MANIFEST.read_text(encoding="utf-8"))
    for row in rows:
        key = Path(row["file"]).name
        if key in {f"arc-orchid-{name}.png" for name in output}:
            image = output[key.removeprefix("arc-orchid-").removesuffix(".png")]
            row.update({
                "width": image.width,
                "height": image.height,
                "source": "generated/arc-orchid-rig-atlas-v2.png",
                "role": "right-facing independent arc aperture" if key.endswith("small-mouth.png") else "right-facing arc orchid firing head",
            })
    MANIFEST.write_text(json.dumps(rows, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print("Exported corrected right-facing Arc Orchid small head, blink and aperture.")


if __name__ == "__main__":
    main()
