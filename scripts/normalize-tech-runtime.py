#!/usr/bin/env python3
"""Place real Tech Garden rig parts onto the native animation canvases.

The generated source pieces intentionally retain generous high-resolution
detail.  Reanimation overrides, however, use the original track dimensions;
feeding the raw 200–300px pieces to a 53px zombie bone makes the part enormous
and shifts its pivot.  This exporter preserves each source PNG and creates a
separate `art/tech/runtime/` set at the exact native canvas sizes.
"""

from __future__ import annotations

import json
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
PARTS = ROOT / "art/tech/parts"
RUNTIME = ROOT / "art/tech/runtime"


def plant_base() -> dict[str, tuple[int, int]]:
    return {"head": (70, 65), "blink": (70, 65), "mouth": (35, 49)}


RUNTIME_PARTS: dict[str, dict[str, tuple[int, int]]] = {
    "sunwheel": {"head": (57, 43), "blink": (57, 43)},
    "pulse-pod": plant_base(),
    "thornshell": {"head": (100, 100), "cracked1": (100, 100), "cracked2": (100, 100)},
    "cable-cap": {"head": (49, 38), "blink": (49, 38), "cap": (81, 53), "stem": (7, 14)},
    "arc-orchid": {"small-head": (30, 30), "small-blink": (30, 30), "small-mouth": (19, 43)},
    "magnet-maw": plant_base(),
    "amber-gourd": plant_base(),
    "scout-bloom": plant_base(),
    "prism-reed": plant_base(),
    "repair-moss": {"head": (57, 43), "blink": (57, 43)},
}

ZOMBIE_RIG = {
    "head": (53, 48),
    "body": (53, 63),
    "body-damage1": (53, 63),
    "body-damage2": (53, 63),
    "jaw": (32, 15),
    "inner-upper": (15, 25),
    "inner-lower": (19, 24),
    "outer-upper": (17, 35),
    "outer-upper-damaged": (17, 35),
    "outer-lower": (19, 28),
    "hand": (25, 27),
    # These are deliberately separate native tracks.  Keeping the old
    # complete-legged zombie image here would make the walk rotate one flat
    # rectangle and was the reason the technology roster looked pasted on.
    "inner-leg-upper": (15, 26),
    "inner-leg-lower": (32, 36),
    "inner-leg-foot": (27, 17),
    "outer-leg-upper": (21, 39),
    "outer-leg-lower": (24, 30),
    "outer-leg-foot": (42, 21),
    "pack": (17, 30),
}

for _slug in (
    "leak-pack", "weld-shield", "turbine-boot", "cable-hook", "wrench-tech",
    "bolt-thrower", "hover-drone", "jammer-aerial", "magnet-salvager", "holo-decoy",
):
    RUNTIME_PARTS[_slug] = dict(ZOMBIE_RIG)

# Turbine Boot deliberately has no chest accessory: its equipment belongs to
# the independently animated boots, and SandboxZombies hides the legacy tie
# track for this one definition.
del RUNTIME_PARTS["turbine-boot"]["pack"]

for _slug in ("weld-shield", "wrench-tech"):
    RUNTIME_PARTS[_slug].update({"prop": (59, 57), "prop-damage1": (59, 57), "prop-damage2": (59, 57)})


def fit_on_native_canvas(source: Path, target: tuple[int, int]) -> Image.Image:
    image = Image.open(source).convert("RGBA")
    target_w, target_h = target
    # Atlas cells are purposely generous, irregular crops.  Preserving their
    # arbitrary aspect ratio here made square bodies land as short 51px
    # sprites inside 53x63 zombie bones and left muzzle tracks visibly
    # detached.  A reanimation override needs to occupy its native canvas;
    # that canvas, rather than atlas cell proportions, defines the bone pivot.
    # Keep one transparent pixel for native squash/stretch, then map both
    # dimensions exactly to the track canvas.
    inner_w, inner_h = max(1, target_w - 2), max(1, target_h - 2)
    resized = image.resize((inner_w, inner_h), Image.Resampling.LANCZOS)
    canvas = Image.new("RGBA", target, (0, 0, 0, 0))
    canvas.alpha_composite(resized, (1 if target_w > 1 else 0, 1 if target_h > 1 else 0))
    return canvas


def main() -> None:
    RUNTIME.mkdir(parents=True, exist_ok=True)
    entries: list[dict[str, object]] = []
    missing: list[str] = []
    for slug, names in RUNTIME_PARTS.items():
        for name, dimensions in names.items():
            filename = f"{slug}-{name}.png"
            source = PARTS / filename
            if not source.is_file():
                missing.append(filename)
                continue
            output = RUNTIME / filename
            image = fit_on_native_canvas(source, dimensions)
            image.save(output)
            entries.append({
                "file": filename,
                "width": dimensions[0],
                "height": dimensions[1],
                "source": f"parts/{filename}",
                "role": "native-rig-override",
            })
    if missing:
        raise SystemExit("Missing real source rig parts: " + ", ".join(sorted(missing)))
    (ROOT / "art/tech/runtime-parts.json").write_text(
        json.dumps(entries, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(f"Exported {len(entries)} native-size rig parts to {RUNTIME}.")


if __name__ == "__main__":
    main()
