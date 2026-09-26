#!/usr/bin/env python3
"""Slice the ten generated mechanical-zombie production atlases into alpha parts.

This intentionally only reads `art/tech/generated/*-rig-atlas-v1.png` and writes
new files below `art/tech/parts/` plus `art/tech/zombie-parts.json`.  It does not
try to invent parts that the atlas did not draw: those are recorded in `missing`.
All source rectangles were visually checked against the atlases; trimming is
alpha-only, so the resulting PNGs retain their original transparent edges.
"""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any, Iterable

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
GENERATED = ROOT / "art" / "tech" / "generated"
OUTPUT = ROOT / "art" / "tech" / "parts"
MANIFEST = ROOT / "art" / "tech" / "zombie-parts.json"
PAD = 2


def part(name: str, bbox: tuple[int, int, int, int], role: str) -> dict[str, Any]:
    return {"name": name, "bbox": bbox, "role": role}


# Rectangles are deliberately a little wider than the component.  `trim_alpha`
# below removes only transparent margin, never opaque pixels or original alpha.
ATLASES: dict[str, dict[str, Any]] = {
    "leak-pack": {
        "source": "leak-pack-rig-atlas-v1.png",
        "parts": [
            part("head", (45, 32, 315, 301), "head"),
            part("jaw", (396, 125, 577, 268), "jaw"),
            part("body", (629, 36, 902, 307), "torso normal"),
            part("body-damage1", (951, 36, 1218, 320), "torso damage stage 1"),
            part("body-damage2", (36, 332, 315, 623), "torso damage stage 2"),
            part("inner-upper", (422, 339, 552, 615), "inner upper arm"),
            part("inner-lower", (724, 343, 827, 609), "inner lower arm"),
            part("outer-upper", (998, 350, 1178, 607), "outer upper arm"),
            part("outer-lower", (111, 664, 251, 913), "outer lower arm"),
            part("inner-hand", (377, 700, 568, 878), "independent inner hand"),
            part("hand", (377, 700, 568, 878), "runtime hand; independent inner hand"),
            part("pack", (629, 619, 916, 927), "battery pack compatibility part"),
            part("battery-pack-back", (629, 619, 916, 927), "battery pack back"),
            part("battery-pack-front", (967, 646, 1203, 911), "battery pack front"),
            part("loose-cable", (35, 983, 322, 1202), "loose power cable"),
            part("power-core", (416, 975, 547, 1215), "glowing removable power core"),
            part("battery-spark-vfx", (670, 953, 848, 1212), "battery spark visual effect"),
            part("battery-steam-vfx", (997, 959, 1226, 1210), "battery vent steam visual effect"),
        ],
    },
    "weld-shield": {
        "source": "weld-shield-rig-atlas-v1.png",
        "parts": [
            part("head", (35, 22, 296, 297), "head"),
            part("jaw", (371, 146, 558, 297), "jaw"),
            part("body", (643, 40, 891, 315), "torso normal"),
            part("body-damage1", (961, 40, 1215, 315), "torso damage stage 1"),
            part("body-damage2", (35, 334, 296, 638), "torso damage stage 2"),
            part("inner-upper", (397, 342, 560, 600), "inner upper arm"),
            part("inner-lower", (705, 381, 861, 578), "inner lower arm"),
            part("outer-upper", (993, 352, 1170, 595), "outer upper arm"),
            part("outer-lower", (60, 659, 258, 903), "outer lower arm"),
            part("inner-hand", (358, 705, 538, 860), "independent inner hand"),
            part("hand", (358, 705, 538, 860), "runtime hand; independent inner hand"),
            part("pack", (644, 632, 895, 915), "shield-back compatibility equipment"),
            part("shield-back", (644, 632, 895, 915), "shield back"),
            part("shield-front", (957, 620, 1225, 919), "shield front"),
            part("prop", (32, 920, 299, 1225), "shield prop normal"),
            part("prop-damage1", (346, 922, 602, 1225), "shield prop damage stage 1"),
            part("prop-damage2", (650, 950, 925, 1198), "shield prop damage stage 2"),
            part("shield-normal", (32, 920, 299, 1225), "detachable shield normal"),
            part("shield-damage1", (346, 922, 602, 1225), "detachable shield damage stage 1"),
            part("shield-damage2", (650, 950, 925, 1198), "detachable shield damage stage 2"),
            part("shield-impact-vfx", (998, 955, 1205, 1215), "shield weld-impact visual effect"),
        ],
    },
    "turbine-boot": {
        "source": "turbine-boot-rig-atlas-v1.png",
        "parts": [
            part("head", (38, 15, 298, 308), "head"),
            part("jaw", (376, 135, 562, 290), "jaw"),
            part("body", (650, 23, 911, 308), "torso normal"),
            part("body-damage1", (959, 19, 1205, 310), "torso damage stage 1"),
            part("body-damage2", (48, 331, 301, 631), "torso damage stage 2"),
            part("inner-upper", (382, 363, 538, 612), "inner upper arm"),
            part("inner-lower", (684, 355, 861, 614), "inner lower arm"),
            part("outer-upper", (1036, 365, 1161, 612), "outer upper arm"),
            part("outer-lower", (68, 651, 282, 931), "outer lower arm"),
            part("inner-hand", (385, 679, 556, 904), "independent inner hand"),
            part("hand", (385, 679, 556, 904), "runtime hand; independent hand"),
            part("leg-left", (707, 659, 836, 935), "left independent leg"),
            part("leg-right", (1000, 646, 1154, 930), "right independent leg"),
            part("turbine-left", (52, 946, 274, 1247), "left boot turbine"),
            part("turbine-right", (351, 953, 563, 1252), "right boot turbine"),
            part("overheat-smoke-vfx", (628, 969, 892, 1257), "turbine overheat smoke"),
            part("dash-flame-vfx", (924, 1006, 1217, 1198), "turbine dash flame"),
        ],
    },
    "cable-hook": {
        "source": "cable-hook-rig-atlas-v1.png",
        "parts": [
            part("head", (44, 70, 297, 324), "head"),
            part("jaw", (331, 201, 512, 334), "jaw"),
            part("body", (564, 83, 856, 344), "torso normal"),
            part("body-damage1", (904, 82, 1192, 344), "torso damage stage 1"),
            part("body-damage2", (1225, 78, 1512, 344), "torso damage stage 2"),
            part("inner-upper", (117, 371, 271, 614), "inner upper arm"),
            part("inner-lower", (346, 377, 471, 597), "inner lower arm"),
            part("inner-hand", (540, 436, 704, 588), "independent inner hand"),
            part("outer-upper", (817, 385, 977, 603), "outer upper arm"),
            part("outer-lower", (1018, 404, 1206, 611), "outer lower arm"),
            part("outer-hand", (1287, 452, 1449, 607), "independent outer hand"),
            part("hand", (1287, 452, 1449, 607), "runtime hand; independent outer hand"),
            part("pack", (40, 640, 376, 965), "winch pack compatibility equipment"),
            part("winch-pack", (40, 640, 376, 965), "winch backpack"),
            part("cable-coil", (443, 686, 648, 943), "coiled cable"),
            part("hook", (667, 738, 949, 864), "detachable hook"),
            part("hook-cast-vfx", (974, 698, 1192, 918), "charged hook cast visual effect"),
            part("hook-sever-vfx", (1223, 694, 1492, 939), "hook sever visual effect"),
        ],
    },
    "wrench-tech": {
        "source": "wrench-tech-rig-atlas-v1.png",
        "parts": [
            part("head", (15, 44, 270, 326), "head"),
            part("jaw", (287, 243, 479, 360), "jaw"),
            part("body", (514, 59, 749, 362), "torso normal"),
            part("body-damage1", (779, 61, 1014, 362), "torso damage stage 1"),
            part("body-damage2", (1045, 61, 1278, 361), "torso damage stage 2"),
            part("inner-upper", (48, 387, 227, 660), "inner upper arm"),
            part("inner-lower", (258, 439, 396, 677), "inner lower arm"),
            part("inner-hand", (438, 493, 613, 670), "independent inner hand"),
            part("outer-upper", (661, 394, 832, 672), "outer upper arm"),
            part("outer-lower", (876, 442, 1029, 662), "outer lower arm"),
            part("outer-hand", (1049, 493, 1256, 671), "independent outer hand"),
            part("hand", (1049, 493, 1256, 671), "runtime hand; independent outer hand"),
            part("pack", (32, 681, 301, 978), "repair tank compatibility equipment"),
            part("repair-tank", (32, 681, 301, 978), "repair tank backpack"),
            part("wrench", (316, 721, 787, 987), "independent oversized wrench"),
            part("prop", (808, 755, 1011, 958), "bucket armour normal"),
            part("prop-damage1", (1068, 758, 1256, 970), "bucket armour damage stage 1"),
            part("bucket-armour", (808, 755, 1011, 958), "bucket armour normal"),
            part("bucket-armour-damaged", (1068, 758, 1256, 970), "bucket armour damaged"),
            part("repair-pulse-vfx", (43, 965, 488, 1218), "repair pulse visual effect"),
        ],
    },
    "bolt-thrower": {
        "source": "bolt-thrower-rig-atlas-v1.png",
        "parts": [
            part("head", (29, 78, 295, 380), "head"),
            part("jaw", (297, 263, 462, 401), "jaw"),
            part("body", (495, 92, 752, 420), "torso normal"),
            part("body-damage1", (797, 91, 1060, 425), "torso damage stage 1"),
            part("body-damage2", (1092, 91, 1352, 427), "torso damage stage 2"),
            part("inner-upper", (44, 461, 234, 728), "inner upper arm"),
            part("inner-lower", (279, 501, 459, 741), "inner lower arm"),
            part("inner-hand", (480, 544, 650, 740), "independent inner hand"),
            part("outer-upper", (711, 462, 903, 741), "outer upper arm"),
            part("outer-lower", (953, 507, 1117, 740), "outer lower arm"),
            part("outer-hand", (1158, 542, 1327, 742), "independent outer hand"),
            part("hand", (1158, 542, 1327, 742), "runtime hand; independent outer hand"),
            part("pack", (32, 768, 317, 1096), "bolt rack compatibility equipment"),
            part("bolt-rack", (32, 768, 317, 1096), "bolt rack backpack"),
            part("torsion-launcher", (334, 819, 699, 1076), "independent torsion bolt launcher"),
            part("bolt-projectile", (737, 893, 947, 1019), "flying bolt projectile"),
            part("bolt-hit-vfx", (956, 819, 1145, 1075), "bolt strike visual effect"),
            part("armour-break", (1212, 846, 1350, 1062), "broken armour debris"),
        ],
    },
    "hover-drone": {
        "source": "hover-drone-rig-atlas-v1.png",
        "parts": [
            part("head", (6, 50, 299, 304), "head"),
            part("jaw", (320, 183, 500, 308), "jaw"),
            part("body", (523, 64, 736, 306), "torso normal"),
            part("body-damage1", (749, 64, 972, 304), "torso damage stage 1"),
            part("body-damage2", (993, 65, 1209, 305), "torso damage stage 2"),
            part("inner-upper", (67, 369, 223, 590), "inner upper arm"),
            part("inner-lower", (352, 386, 447, 592), "inner lower arm"),
            part("outer-upper", (547, 375, 688, 594), "outer upper arm"),
            part("outer-lower", (787, 375, 917, 608), "outer lower arm"),
            part("outer-hand", (1021, 398, 1175, 577), "independent outer hand"),
            part("hand", (1021, 398, 1175, 577), "runtime hand; independent outer hand"),
            part("leg-left", (58, 647, 214, 939), "left independent leg"),
            part("leg-right", (329, 647, 472, 930), "right independent leg"),
            part("pack", (505, 669, 840, 940), "hover ring back compatibility equipment"),
            part("hover-ring-back", (505, 669, 840, 940), "hover ring rear"),
            part("hover-ring-front", (866, 655, 1225, 978), "hover ring front"),
            part("lift-vortex-vfx", (120, 990, 363, 1223), "hover lift vortex"),
            part("landing-dust-vfx", (485, 1011, 731, 1225), "hover landing dust"),
        ],
    },
    "jammer-aerial": {
        "source": "jammer-aerial-rig-atlas-v1.png",
        "parts": [
            part("head", (23, 9, 296, 299), "head"),
            part("jaw", (346, 135, 543, 277), "jaw"),
            part("body", (592, 5, 911, 320), "torso normal"),
            part("body-damage1", (938, 10, 1218, 321), "torso damage stage 1"),
            part("body-damage2", (25, 376, 312, 669), "torso damage stage 2"),
            part("inner-upper", (1008, 386, 1175, 642), "inner upper arm"),
            part("inner-lower", (47, 690, 249, 957), "inner lower arm"),
            part("outer-upper", (383, 721, 563, 947), "outer upper arm"),
            part("outer-lower", (657, 708, 885, 954), "outer lower arm with attached hand"),
            part("hand", (657, 708, 885, 954), "runtime hand; outer forearm and hand are painted as one part"),
            part("leg-left", (978, 678, 1181, 963), "left independent leg"),
            part("leg-right", (78, 970, 290, 1259), "right independent leg"),
            part("pack", (641, 318, 935, 685), "aerial pack back compatibility equipment"),
            part("aerial-dish-front", (319, 296, 636, 671), "aerial dish front"),
            part("aerial-dish-back", (641, 318, 935, 685), "aerial dish back"),
            part("signal-wave-vfx", (317, 976, 652, 1247), "jam signal wave visual effect"),
            part("signal-pulse-vfx", (701, 1028, 934, 1214), "signal pulse visual effect"),
            part("cable-arc-vfx", (981, 974, 1197, 1245), "cable arc visual effect"),
        ],
    },
    "magnet-salvager": {
        "source": "magnet-salvager-rig-atlas-v1.png",
        "parts": [
            part("head", (15, 59, 298, 363), "head"),
            part("jaw", (309, 235, 495, 368), "jaw"),
            part("body", (531, 84, 813, 444), "torso normal"),
            part("body-damage1", (821, 86, 1075, 438), "torso damage stage 1"),
            part("body-damage2", (1087, 81, 1355, 438), "torso damage stage 2"),
            part("magnet-assembly", (22, 416, 327, 777), "independent salvage magnet"),
            part("pack", (22, 416, 327, 777), "magnet assembly compatibility equipment"),
            part("magnet-brace", (339, 427, 538, 777), "magnet brace"),
            part("inner-upper", (554, 469, 745, 728), "inner upper arm"),
            part("inner-lower", (766, 452, 957, 752), "inner lower arm with attached hand"),
            part("outer-upper", (991, 461, 1170, 735), "outer upper arm"),
            part("outer-lower", (1208, 473, 1362, 745), "outer lower arm with attached hand"),
            part("hand", (1208, 473, 1362, 745), "runtime hand; outer forearm and hand are painted as one part"),
            part("leg-left", (26, 799, 252, 1095), "left independent leg"),
            part("leg-right", (311, 786, 547, 1104), "right independent leg"),
            part("magnetic-field-vfx", (534, 793, 835, 1102), "magnetic field visual effect"),
            part("magnet-debris-vfx", (855, 806, 1089, 1084), "magnet debris visual effect"),
            part("magnet-impact-vfx", (1094, 798, 1360, 1085), "magnet impact visual effect"),
        ],
    },
    "holo-decoy": {
        "source": "holo-decoy-rig-atlas-v1.png",
        "parts": [
            part("head", (27, 47, 280, 302), "head"),
            part("jaw", (286, 204, 445, 317), "jaw"),
            part("body", (480, 52, 714, 323), "torso normal"),
            part("body-damage1", (739, 50, 966, 323), "torso damage stage 1"),
            part("body-damage2", (969, 39, 1210, 319), "torso damage stage 2"),
            part("pack", (59, 290, 289, 637), "projector back compatibility equipment"),
            part("projector-back", (59, 290, 289, 637), "projector back"),
            part("projector-front", (405, 332, 636, 617), "projector front"),
            part("inner-upper", (735, 373, 868, 593), "inner upper arm"),
            part("inner-lower", (938, 386, 1151, 613), "inner lower arm with attached hand"),
            part("outer-upper", (110, 656, 254, 892), "outer upper arm"),
            part("outer-lower", (370, 671, 553, 941), "outer lower arm with attached hand"),
            part("hand", (370, 671, 553, 941), "runtime hand; outer forearm and hand are painted as one part"),
            part("leg-left", (649, 634, 847, 940), "left independent leg"),
            part("leg-right", (1013, 634, 1184, 949), "right independent leg"),
            part("holo-ghost", (28, 912, 391, 1257), "hologram ghost"),
            part("holo-glitch-vfx", (434, 965, 769, 1249), "hologram glitch visual effect"),
            part("holo-ready-vfx", (801, 983, 1191, 1247), "hologram deployment visual effect"),
        ],
    },
}


# The canonical system VFX requested by runtime already exist as real parts cut
# from the dedicated system-vfx atlas.  Do not overwrite them here.  These two
# are the only requested canonical VFX unique to the current zombie atlases.
CANONICAL_VFX = [
    {
        "file": "vfx-jam-wave.png",
        "source_slug": "jammer-aerial",
        "bbox": (317, 976, 652, 1247),
        "role": "canonical jammer wave visual effect",
    },
    {
        "file": "vfx-holo-ready.png",
        "source_slug": "holo-decoy",
        "bbox": (801, 983, 1191, 1247),
        "role": "canonical hologram ready visual effect",
    },
]


MISSING: list[dict[str, str]] = []
for _slug in ATLASES:
    MISSING.extend(
        [
            {
                "slug": _slug,
                "runtime": "hat",
                "reason": "No independently painted hat/helmet component; any headwear is joined to the head silhouette.",
            },
            {
                "slug": _slug,
                "runtime": "outer-upper-damaged",
                "reason": "No independently painted damaged outer upper-arm state in this atlas.",
            },
        ]
    )
MISSING.extend(
    [
        {
            "slug": "turbine-boot",
            "runtime": "pack",
            "reason": "No separate backpack component was painted; the only separated equipment is the pair of boot turbines.",
        },
        {
            "slug": "wrench-tech",
            "runtime": "prop-damage2",
            "reason": "Only normal and one damaged bucket-armour state were painted.",
        },
    ]
)

# These have a valid runtime `-hand.png`, but the source deliberately joins that
# hand to the forearm.  Keeping this separate from `missing` makes it clear that
# runtime has an honest visual file while a standalone palm was not invented.
LIMITATIONS = [
    {
        "slug": "jammer-aerial",
        "part": "hand",
        "reason": "Both painted hands are attached to their forearms; the runtime hand file uses the real outer forearm-and-hand part.",
    },
    {
        "slug": "magnet-salvager",
        "part": "hand",
        "reason": "Both painted hands are attached to their forearms; the runtime hand file uses the real outer forearm-and-hand part.",
    },
    {
        "slug": "holo-decoy",
        "part": "hand",
        "reason": "Both painted hands are attached to their forearms; the runtime hand file uses the real outer forearm-and-hand part.",
    },
]


def trim_alpha(image: Image.Image, bbox: tuple[int, int, int, int]) -> Image.Image:
    """Extract and tightly trim one visual component while retaining RGBA alpha."""
    crop = image.crop(bbox)
    alpha_bbox = crop.getchannel("A").getbbox()
    if alpha_bbox is None:
        raise ValueError(f"empty source crop: {bbox}")
    left, top, right, bottom = alpha_bbox
    left = max(0, left - PAD)
    top = max(0, top - PAD)
    right = min(crop.width, right + PAD)
    bottom = min(crop.height, bottom + PAD)
    return crop.crop((left, top, right, bottom))


def save_part(
    image: Image.Image,
    source_rel: str,
    slug: str,
    item: dict[str, Any],
    file_name: str,
) -> dict[str, Any]:
    out = trim_alpha(image, item["bbox"])
    path = OUTPUT / file_name
    out.save(path, "PNG")
    return {
        "file": f"parts/{file_name}",
        "width": out.width,
        "height": out.height,
        "source": f"{source_rel}#{','.join(map(str, item['bbox']))}",
        "role": item["role"],
        "slug": slug,
    }


def main() -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    manifest_parts: list[dict[str, Any]] = []
    images: dict[str, Image.Image] = {}

    for slug, data in ATLASES.items():
        source_rel = f"generated/{data['source']}"
        image = Image.open(GENERATED / data["source"]).convert("RGBA")
        images[slug] = image
        for item in data["parts"]:
            file_name = f"{slug}-{item['name']}.png"
            manifest_parts.append(save_part(image, source_rel, slug, item, file_name))

    for item in CANONICAL_VFX:
        slug = item["source_slug"]
        source = ATLASES[slug]["source"]
        source_rel = f"generated/{source}"
        manifest_parts.append(save_part(images[slug], source_rel, slug, item, item["file"]))

    payload = {
        "schema": 1,
        "description": "True RGBA parts cut from the ten mechanical-zombie rig atlases; no old expansion or placeholder art.",
        "parts": manifest_parts,
        "existingCanonicalVfx": [
            "vfx-battery-core.png",
            "vfx-turbine-burst.png",
            "vfx-hook-snap.png",
            "vfx-hook-line.png",
            "vfx-repair-pulse.png",
            "vfx-bolt-ready.png",
            "vfx-bolt-shot.png",
            "vfx-bolt-hit-0.png",
            "vfx-drone-hum.png",
            "vfx-magnet-field.png",
            "vfx-holo-glitch.png",
        ],
        "limitations": LIMITATIONS,
        "missing": MISSING,
    }
    MANIFEST.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n")
    print(f"wrote {len(manifest_parts)} parts to {OUTPUT}")
    print(f"recorded {len(MISSING)} components that the atlases do not separately draw")


if __name__ == "__main__":
    main()
