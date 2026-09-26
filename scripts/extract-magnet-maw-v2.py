#!/usr/bin/env python3
"""Export the corrected right-facing Magnet Maw rig from its v2 source atlas.

This is a crop-only production step.  The v1 atlas remains retained for art
history; the three runtime parts below come from the separately generated v2
atlas and are recorded against that exact source.
"""

from __future__ import annotations

import json
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "art/tech/generated/magnet-maw-rig-atlas-v2.png"
PARTS = ROOT / "art/tech/parts"
MANIFEST = ROOT / "art/tech/plant-parts.json"

# These are alpha-component bounds checked on the real transparent atlas.
SPECS = {
    "head": (82, 49, 456, 404),
    "blink": (564, 96, 936, 412),
    # The open C-shaped magnetic jaw is the independently animated nozzle.
    "mouth": (1061, 108, 1314, 395),
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
    PARTS.mkdir(parents=True, exist_ok=True)
    output: dict[str, Image.Image] = {}
    for name, box in SPECS.items():
        part = atlas.crop(box)
        output[name] = trim(part)
        output[name].save(PARTS / f"magnet-maw-{name}.png", optimize=True)

    rows = json.loads(MANIFEST.read_text(encoding="utf-8"))
    for row in rows:
        key = Path(row["file"]).name
        if key in {f"magnet-maw-{name}.png" for name in output}:
            image = output[key.removeprefix("magnet-maw-").removesuffix(".png")]
            row.update({
                "width": image.width,
                "height": image.height,
                "source": "generated/magnet-maw-rig-atlas-v2.png",
                "role": "right-facing magnetic head" if key.endswith("head.png") else "right-facing detached magnetic rig part",
            })
    MANIFEST.write_text(json.dumps(rows, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print("Exported corrected right-facing Magnet Maw head, blink and mouth.")


if __name__ == "__main__":
    main()
