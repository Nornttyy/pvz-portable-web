#!/usr/bin/env python3
"""Export the corrected right-facing Amber Gourd runtime rig from v2 art."""

from __future__ import annotations

import json
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "art/tech/generated/amber-gourd-rig-atlas-v2.png"
PARTS = ROOT / "art/tech/parts"
MANIFEST = ROOT / "art/tech/plant-parts.json"

SPECS = {
    "head": (37, 53, 424, 430),
    "blink": (461, 81, 838, 427),
    # The open lip is used by the native mouth track; it contains the full
    # organic resin nozzle rather than a pasted cannon barrel.
    "mouth": (1189, 228, 1410, 377),
}


def trim(image: Image.Image, padding: int = 2) -> Image.Image:
    box = image.getchannel("A").getbbox()
    if box is None:
        raise ValueError("empty alpha crop")
    item = image.crop(box)
    result = Image.new("RGBA", (item.width + padding * 2, item.height + padding * 2))
    result.alpha_composite(item, (padding, padding))
    return result


def main() -> None:
    atlas = Image.open(SOURCE).convert("RGBA")
    output: dict[str, Image.Image] = {}
    for name, box in SPECS.items():
        item = atlas.crop(box)
        output[name] = trim(item)
        output[name].save(PARTS / f"amber-gourd-{name}.png", optimize=True)

    rows = json.loads(MANIFEST.read_text(encoding="utf-8"))
    for row in rows:
        key = Path(row["file"]).name
        if key in {f"amber-gourd-{name}.png" for name in output}:
            image = output[key.removeprefix("amber-gourd-").removesuffix(".png")]
            row.update({
                "width": image.width,
                "height": image.height,
                "source": "generated/amber-gourd-rig-atlas-v2.png",
                "role": "right-facing amber head" if key.endswith("head.png") else "right-facing detached resin nozzle rig part",
            })
    MANIFEST.write_text(json.dumps(rows, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print("Exported corrected right-facing Amber Gourd head, blink and mouth.")


if __name__ == "__main__":
    main()
