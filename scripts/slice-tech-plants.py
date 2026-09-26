#!/usr/bin/env python3
"""Slice only the generated tech-plant rig atlases into RGBA PNG rig pieces.

The atlas pixels stay unpainted: every exported file is a crop of an existing
alpha-bearing region from art/tech/generated.  The table below is deliberately
manual because an effect such as a burst can have several disconnected alpha
islands that still belong to one usable sprite.
"""

from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path

from PIL import Image
from PIL import ImageDraw


ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIR = ROOT / "art" / "tech" / "generated"
OUT_DIR = ROOT / "art" / "tech" / "parts"
MANIFEST = ROOT / "art" / "tech" / "plant-parts.json"


@dataclass(frozen=True)
class Part:
    suffix: str
    box: tuple[int, int, int, int]
    role: str
    # Elliptical alpha cut-outs only remove an attached muzzle/face from the
    # source crop.  They never add painted pixels, and let the matching mouth
    # piece sit above the head as a real skeletal layer.
    holes: tuple[tuple[int, int, int, int], ...] = ()


# Box coordinates were checked against the actual transparent atlas pixels.
# Names used by SandboxPlants (head/blink/mouth; small-* for Arc Orchid;
# cap/stem for Cable Cap; cracked1/cracked2 for Thornshell) are intentionally
# retained verbatim so they can be registered by the resource pack unchanged.
ATLAS: dict[str, tuple[Part, ...]] = {
    "sunwheel": (
        Part("petal-back", (18, 6, 392, 366), "back petal wheel"),
        Part("body", (411, 96, 727, 357), "leafy body"),
        Part("head", (760, 69, 1063, 335), "normal face"),
        Part("blink", (1119, 61, 1427, 335), "closed-eye face"),
        Part("petal-front", (14, 365, 411, 737), "front petal ring"),
        Part("stem", (475, 395, 714, 720), "stem"),
        Part("leaf-back", (793, 401, 1048, 718), "back leaf"),
        Part("leaf-front", (1153, 403, 1402, 712), "front leaf"),
        Part("collector-disk", (39, 743, 348, 1018), "collector disk"),
        Part("body-damage1", (403, 744, 708, 1016), "collector damage stage 1"),
        Part("body-damage2", (761, 743, 1070, 1015), "collector damage stage 2"),
        Part("charge-ring", (1102, 724, 1429, 1020), "root-network charge ring"),
    ),
    "pulse-pod": (
        Part("head", (21, 20, 343, 327), "normal pod head with its integrated living nozzle"),
        Part("blink", (350, 64, 619, 309), "closed-eye pod head with its integrated living nozzle"),
        Part("mouth", (654, 132, 796, 302), "detached closed muzzle ring"),
        Part("mouth-open", (868, 127, 1042, 318), "detached charged muzzle"),
        Part("stem", (51, 336, 247, 622), "flexible stem"),
        Part("body", (283, 348, 545, 624), "pod torso"),
        Part("leaf-back", (564, 435, 807, 614), "back leaf"),
        Part("leaf-front", (837, 435, 1112, 614), "front leaf"),
        Part("root-base", (14, 676, 298, 866), "root base"),
        Part("pulse-projectile-0", (308, 700, 532, 846), "pulse seed flight frame 0"),
        Part("pulse-projectile-1", (554, 704, 798, 849), "pulse seed flight frame 1"),
        Part("pulse-projectile-2", (810, 696, 1106, 847), "pulse seed flight frame 2"),
        Part("pulse-muzzle", (14, 890, 309, 1135), "pulse muzzle flash"),
        Part("pulse-hit", (323, 883, 559, 1133), "pulse impact"),
        Part("charge-halo", (577, 911, 811, 1138), "resonance charge halo"),
        Part("capacitor-crack", (824, 864, 1111, 1126), "damaged capacitor head"),
        Part("leaf-torn", (35, 1154, 266, 1371), "torn leaf damage overlay"),
        Part("leaf-left", (318, 1174, 545, 1370), "detached leaf left"),
        Part("leaf-right", (577, 1174, 798, 1371), "detached leaf right"),
        Part("root-damaged", (838, 1210, 1102, 1374), "damaged root base"),
    ),
    "thornshell": (
        Part("head", (8, 115, 311, 429), "normal shell face"),
        Part("blink", (302, 115, 603, 428), "closed-eye shell face"),
        Part("mouth", (640, 230, 803, 369), "detached face and mouth"),
        Part("shell-front", (835, 154, 1117, 407), "front shell plate"),
        Part("shell-back", (21, 485, 320, 705), "back shell plate"),
        Part("frond-left", (334, 471, 603, 712), "left striking frond"),
        Part("frond-right", (651, 472, 858, 720), "right striking frond"),
        Part("root-base", (878, 536, 1110, 700), "root base"),
        Part("thorn-strike-0", (15, 739, 335, 971), "thorn strike frame 0"),
        Part("thorn-strike-1", (321, 753, 625, 980), "thorn strike frame 1"),
        Part("thorn-projectile-0", (631, 836, 821, 937), "thorn projectile frame 0"),
        Part("thorn-projectile-1", (837, 854, 990, 929), "thorn projectile frame 1"),
        Part("thorn-projectile-2", (996, 865, 1112, 922), "thorn projectile frame 2"),
        Part("thorn-hit", (35, 995, 284, 1285), "thorn impact burst"),
        Part("thorn-resonance", (308, 981, 565, 1290), "defense resonance halo"),
        Part("cracked1", (582, 1048, 873, 1282), "cracked shell damage state"),
        Part("cracked2", (876, 1033, 1116, 1281), "torn frond damage state"),
    ),
    "cable-cap": (
        Part("head", (13, 22, 305, 299), "normal mushroom cap face"),
        Part("blink", (299, 22, 591, 299), "closed-eye mushroom cap face"),
        Part("mouth", (595, 154, 797, 292), "detached face/mouth plate"),
        Part("cap", (823, 65, 1113, 282), "cap top"),
        Part("cap-underside", (10, 361, 299, 507), "cap underside"),
        Part("node-left", (347, 353, 506, 553), "left cable node"),
        Part("node-center", (615, 359, 757, 540), "center cable node"),
        Part("node-right", (910, 361, 1075, 553), "right cable node"),
        Part("stem", (12, 552, 284, 864), "coiled mycelium stem"),
        Part("root-base", (298, 632, 602, 853), "root base"),
        Part("cable-left", (630, 621, 853, 810), "left root cable"),
        Part("cable-right", (892, 604, 1112, 830), "right root cable"),
        Part("cable-loop-left", (37, 873, 261, 1103), "curled root cable left"),
        Part("cable-loop-right", (307, 903, 555, 1098), "curled root cable right"),
        Part("hub-idle", (599, 934, 818, 1060), "idle hub glow"),
        Part("hub-full", (728, 1135, 905, 1378), "full hub glow"),
        Part("root-link", (20, 1155, 445, 1355), "root link pulse"),
        Part("root-sever", (458, 1163, 697, 1360), "severed cable spark"),
        Part("root-repair", (919, 1141, 1113, 1367), "reconnected cable spark"),
        Part("cable-fray", (861, 868, 1100, 1106), "cracked cable node"),
    ),
    "arc-orchid": (
        Part("head", (36, 5, 415, 423), "normal orchid body/head"),
        Part("small-head", (480, 153, 717, 387), "small firing head"),
        Part("small-blink", (808, 153, 1047, 387), "small firing head alternate"),
        Part("small-mouth", (1151, 540, 1349, 666), "detached aperture muzzle"),
        Part("stem", (1052, 70, 1442, 433), "long orchid stem"),
        Part("petal-left", (91, 440, 331, 704), "left detached petal"),
        Part("petal-center", (413, 418, 681, 707), "center detached petal"),
        Part("petal-right", (786, 448, 1026, 711), "right detached petal"),
        Part("aperture-closed", (480, 153, 717, 387), "closed arc aperture"),
        Part("aperture-open", (808, 153, 1047, 387), "open arc aperture"),
        Part("body-damage1", (32, 705, 386, 1060), "scorched orchid damage state 1"),
        Part("body-damage2", (424, 705, 772, 1062), "scorched orchid damage state 2"),
        Part("arc-bolt", (796, 840, 1119, 989), "arc projectile"),
        Part("arc-hit", (1150, 760, 1410, 1033), "arc impact burst"),
    ),
    "magnet-maw": (
        Part("head", (75, 32, 412, 397), "normal trap head with integrated magnetic throat"),
        Part("blink", (810, 495, 989, 641), "detached blink eye"),
        Part("mouth", (66, 531, 313, 657), "detached lower magnetic jaw"),
        Part("upper-maw-open", (445, 77, 704, 377), "open upper magnetic jaw"),
        Part("upper-maw-closed", (773, 138, 1033, 349), "closed upper magnetic jaw"),
        Part("lower-maw", (1115, 173, 1402, 291), "separate lower jaw"),
        Part("stem", (376, 391, 775, 709), "trap stem and leaf base"),
        Part("body-damage1", (1092, 388, 1410, 714), "cracked trap damage state"),
        Part("body-damage2", (43, 726, 366, 1044), "heavily cracked trap damage state"),
        Part("magnet-pull", (395, 802, 696, 1002), "magnetic pull effect"),
        Part("magnet-inhale", (732, 717, 1093, 1044), "magnetic inhale effect"),
        Part("armor-strip", (1103, 731, 1416, 1048), "armor strip effect"),
    ),
    "amber-gourd": (
        Part("head", (39, 43, 418, 442), "normal amber gourd head with integrated resin nozzle"),
        Part("blink", (834, 597, 1038, 667), "blink eyelid overlay"),
        Part("mouth", (450, 193, 715, 378), "detached closed nozzle lip"),
        Part("mouth-open", (784, 205, 1038, 382), "detached open nozzle lip"),
        Part("top-leaves", (1082, 136, 1421, 385), "gourd top leaves"),
        Part("leaf-back", (32, 569, 395, 733), "back leaf"),
        Part("stem", (432, 431, 798, 753), "vine stem and leaf base"),
        Part("body-damage1", (1097, 433, 1418, 745), "scratched gourd damage state"),
        Part("body-damage2", (32, 750, 348, 1052), "cracked gourd damage state"),
        Part("resin-glob", (394, 846, 722, 998), "resin projectile"),
        Part("resin-splash", (782, 800, 978, 1024), "resin splash impact"),
        Part("resin-slow", (1051, 846, 1424, 1023), "resin slow puddle"),
    ),
    "scout-bloom": (
        Part("head", (17, 42, 268, 317), "normal scout head with integrated aim aperture"),
        Part("blink", (259, 42, 511, 318), "closed-eye scout head with integrated aim aperture"),
        Part("aim-left", (505, 74, 742, 314), "left aim pose"),
        Part("aim-up", (734, 46, 969, 329), "up aim pose"),
        Part("mouth", (964, 117, 1105, 261), "detached aiming aperture"),
        Part("stem", (52, 330, 201, 611), "twist stem"),
        Part("body", (246, 327, 429, 616), "body stem"),
        Part("leaf-left", (432, 437, 635, 611), "left leaf"),
        Part("leaf-right", (659, 448, 873, 607), "right leaf"),
        Part("root-base", (871, 395, 1114, 622), "root base"),
        Part("scout-projectile-0", (46, 700, 265, 796), "guided seed frame 0"),
        Part("scout-projectile-1", (319, 678, 563, 800), "guided seed frame 1"),
        Part("scout-lock", (261, 875, 579, 1028), "target lock arc"),
        Part("scout-hit", (69, 891, 227, 1041), "guided seed impact"),
        Part("scout-muzzle", (619, 844, 859, 1065), "scout muzzle flash"),
        Part("body-damage1", (899, 649, 1099, 850), "sensor burst damage"),
        Part("body-damage2", (859, 1135, 1090, 1344), "torn leaf damage state"),
    ),
    "prism-reed": (
        Part("head", (23, 9, 216, 340), "normal prism head with integrated crystal aperture"),
        Part("blink", (251, 37, 448, 341), "closed shutter head with integrated crystal aperture"),
        Part("prism", (500, 57, 626, 337), "detached prism crystal"),
        Part("mouth", (693, 157, 858, 325), "detached aperture ring"),
        Part("aim-right", (905, 108, 1145, 345), "right aim head"),
        Part("stem-upper", (295, 351, 413, 688), "upper reed stem"),
        Part("stem-lower", (532, 469, 656, 689), "lower reed stem"),
        Part("leaf-left", (736, 446, 881, 687), "left reed leaf"),
        Part("leaf-right", (943, 421, 1107, 687), "right reed leaf"),
        Part("root-base", (1, 750, 258, 984), "root base"),
        Part("prism-beam-tail", (286, 837, 458, 896), "beam tail segment"),
        Part("prism-beam-mid", (479, 835, 682, 898), "beam middle segment"),
        Part("prism-beam-core", (690, 834, 947, 898), "beam core segment"),
        Part("prism-charge", (339, 1047, 588, 1317), "prism charge flare"),
        Part("prism-hit", (947, 729, 1139, 969), "prism impact flare"),
        Part("body-damage1", (631, 989, 844, 1342), "cracked prism damage state"),
        Part("body-damage2", (934, 997, 1116, 1352), "burned reed damage state"),
    ),
    "repair-moss": (
        Part("head", (3, 66, 322, 374), "normal moss face with integrated smile"),
        Part("blink", (332, 117, 607, 352), "closed-eye moss face"),
        Part("mouth", (692, 226, 789, 287), "detached smiling mouth"),
        Part("moss-front", (865, 156, 1114, 337), "front moss tuft"),
        Part("moss-back", (10, 414, 303, 631), "back moss cushion"),
        Part("tendril-left", (330, 391, 566, 610), "left repair tendril"),
        Part("tendril-right", (614, 410, 842, 606), "right repair tendril"),
        Part("tendril-long", (854, 390, 1116, 619), "long reaching tendril"),
        Part("leaf-left", (45, 695, 252, 849), "left leaf pad"),
        Part("leaf-right", (325, 696, 519, 830), "right leaf pad"),
        Part("root-base", (573, 656, 843, 861), "root pad"),
        Part("repair-spore", (946, 660, 1056, 837), "healing sap bead"),
        Part("repair-thread-0", (31, 918, 275, 1071), "repair thread segment 0"),
        Part("repair-thread-1", (324, 909, 564, 1080), "repair thread segment 1"),
        Part("repair-thread-2", (593, 921, 832, 1064), "repair thread segment 2"),
        Part("repair-cast", (905, 893, 1082, 1090), "repair cast sparkle"),
        Part("repair-stitch", (591, 1142, 831, 1326), "reconnect knot effect"),
        Part("body-damage1", (869, 1168, 1086, 1307), "scorched moss damage state"),
        Part("tendril-fray", (333, 1136, 546, 1347), "frayed tendril damage state"),
    ),
}


def trim_alpha(image: Image.Image, padding: int = 2) -> Image.Image:
    """Trim transparent margins while preserving semitransparent painted glows."""
    alpha = image.getchannel("A")
    bbox = alpha.getbbox()
    if bbox is None:
        raise ValueError("crop contains no alpha pixels")
    left, top, right, bottom = bbox
    piece = image.crop((left, top, right, bottom))
    if not padding:
        return piece
    result = Image.new("RGBA", (piece.width + padding * 2, piece.height + padding * 2))
    result.alpha_composite(piece, (padding, padding))
    return result


def remove_attached_holes(image: Image.Image, source_box: tuple[int, int, int, int], holes: tuple[tuple[int, int, int, int], ...]) -> None:
    """Remove only designated attached tool pixels from a source crop's alpha."""
    if not holes:
        return
    alpha = image.getchannel("A")
    draw = ImageDraw.Draw(alpha)
    source_left, source_top, _, _ = source_box
    for left, top, right, bottom in holes:
        draw.ellipse((left - source_left, top - source_top, right - source_left, bottom - source_top), fill=0)
    image.putalpha(alpha)


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest: list[dict[str, object]] = []
    seen: set[str] = set()
    for slug, pieces in ATLAS.items():
        source_name = f"{slug}-rig-atlas-v1.png"
        source_path = SOURCE_DIR / source_name
        if not source_path.is_file():
            raise FileNotFoundError(source_path)
        atlas = Image.open(source_path).convert("RGBA")
        for part in pieces:
            filename = f"{slug}-{part.suffix}.png"
            if filename in seen:
                raise ValueError(f"duplicate export name: {filename}")
            seen.add(filename)
            left, top, right, bottom = part.box
            if left < 0 or top < 0 or right > atlas.width or bottom > atlas.height:
                raise ValueError(f"{filename}: crop lies outside {source_name}: {part.box}")
            source_crop = atlas.crop(part.box)
            remove_attached_holes(source_crop, part.box, part.holes)
            piece = trim_alpha(source_crop)
            if piece.getchannel("A").getbbox() is None:
                raise ValueError(f"{filename}: empty alpha crop")
            output_path = OUT_DIR / filename
            piece.save(output_path, format="PNG", optimize=True)
            manifest.append({
                "file": f"parts/{filename}",
                "width": piece.width,
                "height": piece.height,
                "source": f"generated/{source_name}",
                "role": part.role,
            })
    MANIFEST.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"wrote {len(manifest)} transparent plant parts to {OUT_DIR}")


if __name__ == "__main__":
    main()
