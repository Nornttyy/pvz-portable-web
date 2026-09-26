#!/usr/bin/env python3
"""Export the generated missing plant pieces as independent transparent PNGs.

This is deliberately a crop/trim/downsample pass only: every pixel comes from
one of the dedicated transparent image-generation atlases in missing-atlases/.
It does not redraw, recolour, or composite game art.
"""

from __future__ import annotations

import json
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parent
ATLAS_DIR = ROOT / "missing-atlases"
PARTS_DIR = ROOT / "parts"
MANIFEST_PATH = ROOT / "plant-missing-parts.json"

# (output filename, source atlas, manual atlas cell, longest output edge, role)
# The two-cell atlas slices intentionally keep a clear transparent gutter between
# source components before alpha trimming, so no unrelated part is carried over.
EXPORTS = (
    (
        "cable-cap-body-damage1.png",
        "cable-cap-damage-atlas-v1.png",
        (0, 0, 887, 887),
        360,
        "cable cap root-and-mycelium damage state 1; no head, cap, or stem",
    ),
    (
        "cable-cap-body-damage2.png",
        "cable-cap-damage-atlas-v1.png",
        (887, 0, 1774, 887),
        360,
        "cable cap root-and-mycelium damage state 2; no head, cap, or stem",
    ),
    (
        "repair-moss-body-damage2.png",
        "repair-moss-damage2-atlas-v1.png",
        None,
        360,
        "repair moss heavy body damage state 2; no face or tendril",
    ),
    (
        "sunwheel-sun.png",
        "sunwheel-source-atlas-v1.png",
        (0, 0, 999, 787),
        260,
        "detached sunwheel sunlight token",
    ),
    (
        "root-charge-source.png",
        "sunwheel-source-atlas-v1.png",
        (999, 0, 1999, 787),
        320,
        "detached root-network charge source",
    ),
    (
        "pulse-triple.png",
        "pulse-triple-atlas-v1.png",
        None,
        360,
        "pulse pod three-shot pulse projectile fan",
    ),
    (
        "thorn-heal.png",
        "thorn-heal-atlas-v1.png",
        None,
        360,
        "thornshell thorn-and-leaf restoration effect",
    ),
    (
        "arc-chain.png",
        "arc-chain-atlas-v1.png",
        (0, 0, 1100, 887),
        420,
        "arc orchid branching chain-lightning segment",
    ),
    (
        "arc-chain-hit.png",
        "arc-chain-atlas-v1.png",
        (1100, 0, 1774, 887),
        300,
        "arc orchid chain-lightning impact bloom",
    ),
    (
        "amber-gourd-resin-muzzle.png",
        "amber-gourd-resin-muzzle-atlas-v1.png",
        None,
        360,
        "amber gourd resin firing muzzle flash",
    ),
    (
        "scout-bloom-scout-projectile-2.png",
        "scout-bloom-projectile-2-atlas-v1.png",
        None,
        300,
        "scout bloom guided seed projectile flight frame 2",
    ),
    (
        "prism-reed-lens-glint.png",
        "prism-reed-lens-glint-atlas-v1.png",
        None,
        300,
        "prism reed detached lens glint; distinct from vfx-prism-muzzle",
    ),
    (
        "repair-moss-repair-cleanse.png",
        "repair-moss-cleanse-atlas-v1.png",
        None,
        330,
        "repair moss cleansing repair pulse",
    ),
)


def alpha_trim(image: Image.Image, padding: int = 2) -> Image.Image:
    """Crop only transparent exterior, retaining a small transparent rig margin."""
    rgba = image.convert("RGBA")
    bounds = rgba.getchannel("A").getbbox()
    if bounds is None:
        raise ValueError("source cell has no opaque or translucent pixels")
    left, top, right, bottom = bounds
    left = max(0, left - padding)
    top = max(0, top - padding)
    right = min(rgba.width, right + padding)
    bottom = min(rgba.height, bottom + padding)
    return rgba.crop((left, top, right, bottom))


def scale_to_longest_edge(image: Image.Image, longest_edge: int) -> Image.Image:
    if max(image.size) <= longest_edge:
        return image
    ratio = longest_edge / max(image.size)
    target = tuple(max(1, round(dimension * ratio)) for dimension in image.size)
    return image.resize(target, Image.Resampling.LANCZOS)


def export_part(filename: str, atlas_name: str, crop: tuple[int, int, int, int] | None,
                longest_edge: int, role: str) -> dict[str, object]:
    source_path = ATLAS_DIR / atlas_name
    if not source_path.is_file():
        raise FileNotFoundError(source_path)
    image = Image.open(source_path).convert("RGBA")
    if crop is not None:
        image = image.crop(crop)
    image = scale_to_longest_edge(alpha_trim(image), longest_edge)
    output_path = PARTS_DIR / filename
    image.save(output_path, format="PNG", optimize=True)

    # The manifest is intentionally restricted to the actual newly exported art.
    reloaded = Image.open(output_path).convert("RGBA")
    alpha = reloaded.getchannel("A")
    if alpha.getbbox() is None or alpha.getextrema()[0] >= 255:
        raise ValueError(f"{output_path} is not a transparent usable PNG")
    return {
        "file": f"parts/{filename}",
        "width": reloaded.width,
        "height": reloaded.height,
        "source": f"missing-atlases/{atlas_name}",
        "role": role,
    }


def main() -> None:
    PARTS_DIR.mkdir(parents=True, exist_ok=True)
    manifest = [export_part(*item) for item in EXPORTS]
    MANIFEST_PATH.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n")
    print(f"exported {len(manifest)} independent RGBA parts")
    for item in manifest:
        print(f"{item['file']}: {item['width']}x{item['height']}")


if __name__ == "__main__":
    main()
