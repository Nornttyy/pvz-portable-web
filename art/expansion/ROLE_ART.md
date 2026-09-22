# Native-derived role art

All new raster artwork was made with the built-in `image_gen` editing tool, not a procedural drawing substitute. Registration scripts only extract alpha-separated pieces, resize to native bone canvases, and copy explicitly reused native parts.

## Zombie correction (2026-09-23)

Source parts were found in the existing audited resource pack (`resources/source/reanim` in the local preview). `Zombie.reanim` provides actual walking, eating and damage poses. The original files remain unchanged in the resource bundle.

- Source contact sheet: `zombie-native-reference/base.png`.
- Exact prompts and the 12 costume specifications: `zombie-native-redraw.json`.
- Selected generated atlases: `zombie-native-generated/<variant>.png`.
- All 12 faces and lower jaws reuse the original pixel data byte-for-byte. Native hands and legs retain their original animation tracks.
- Torso, near/far upper arms, near/far forearms, and a damaged upper arm are separate parts for every variant.
- Hats use a separate native hair track. Hats are no longer included when fitting the head, preventing a squashed face.
- Armor sources and identities: `zombie-armor-redraw.json`; the final prompt is `prompt-zombie-armor-final.txt`. The initial two-row drafts were rejected because they edited the wrong row and had opaque backgrounds.
- Selected three-state armor strips: `zombie-armor-generated/<variant>.png`; all three states register to original cone/bucket canvases.
- Final runtime parts: `parts/`. Reproduce their registration with `scripts/register-zombie-redraw.swift <repo> <native-resource-root>`.
- `zombie-poses-proof.png` is an **offline skeleton-pose proof, not a browser screenshot**. Each variant shows walking, eating/aiming, and a missing-arm pose using the native transforms.

Runtime damage hooks retain custom damaged sleeves and armor even after native damage code changes the image. Detached armor particles also use the matching custom damaged item. Repairing a helmet restores its matching less-damaged image.

## Growth mushroom and muzzle correction

- Three mushroom stages and ranged role/VFX prompts: `role-redraw.json`, `prompt-role-corrections.txt`, `prompt-ranged-vfx.txt`.
- Selected mushroom sources: `role-generated/storm-mushroom-{0,1,2}-fixed.png`; face, blink, cap and stem are separate.
- Ranged projectile/effect source: `role-generated/vfx-ranged.png`.
- Triple-shooter mouth pairs: `prompts-triple-seams.json`, `role-generated/{fire,ice}-triple.png`.
- Other matching mouth sets: `prompts-mouth-seams.json`, `seam-generated/`.
- Native-derived initial pea redraw: `generated/native-seams.png`, `generated/native-seams-blink.png` and matching prompt text files.
- `mouth-joints-audit.json` checks alpha overlap through native animation frames. This is a geometric check, not live gameplay QA.

The growth mushroom, ranged zombies and all custom art remain sandbox content in this release. Adventure unlocks are not yet connected; original adventure progress is unaffected.
