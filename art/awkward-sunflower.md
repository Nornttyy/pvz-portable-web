# 尬葵表情素材

- Mode: built-in image generation tool, precise-object-edit.
- Edit reference: original `reanim/SunFlower_head.png` (57 × 43).
- Final asset: `addons/art/awkward-sunflower-face.png` (57 × 43 RGBA).
- Only the non-smiling face is new. Petals, leaves, stalk, blink and sweat reuse original rig parts. The original face remains untouched and is used when not embarrassed.
- Preparation: `scripts/prepare-awkward-face.mjs` trims alpha padding and downsamples; no procedural redrawing.
- Runtime: only this new face PNG is embedded, not retired addon art.

## Final prompt

Use case: precise-object-edit. Asset type: a tiny 2D game rig facial sprite, not a whole character. Input image 1 is the edit target: original SunFlower_head.png. Change ONLY the curved smiling mouth into a short flat slightly awkward closed mouth; the character is embarrassed and no longer smiling. Preserve the exact oval brown sunflower face silhouette, eye placement and small black eyes, muted brown colors, simple softly shaded low-resolution original Plants vs Zombies cartoon style, thin dark outer outline, frontal camera, horizontal oval proportions. No petals, leaves, body, sweat, blush, eyebrows, text or additional accessories. Transparent background with actual alpha, no checkerboard painted in. One isolated oval face filling the canvas, no white border, suitable for scaling back to the original 58 by 43 pixel bone texture. Do not make it glossy, realistic, more detailed, more saturated, or a sticker.
