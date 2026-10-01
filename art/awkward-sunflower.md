# 尬葵表情素材

- Mode: built-in image generation tool, precise-object-edit.
- Edit reference: original `reanim/SunFlower_head.png` (57 × 43).
- Final asset: `addons/art/awkward-sunflower-face.png` (57 × 43 RGBA).
- The non-smiling face and single blue embarrassment drop are new. Petals, leaves, stalk and blink reuse original rig parts. The original face remains untouched and is used when not embarrassed.
- Preparation: `scripts/prepare-awkward-face.mjs` trims alpha padding and downsamples; no procedural redrawing.
- Runtime: only the new face and blue drop PNGs are embedded, not retired addon art.

## Final prompt

Use case: precise-object-edit. Asset type: a tiny 2D game rig facial sprite, not a whole character. Input image 1 is the edit target: original SunFlower_head.png. Change ONLY the curved smiling mouth into a short flat slightly awkward closed mouth; the character is embarrassed and no longer smiling. Preserve the exact oval brown sunflower face silhouette, eye placement and small black eyes, muted brown colors, simple softly shaded low-resolution original Plants vs Zombies cartoon style, thin dark outer outline, frontal camera, horizontal oval proportions. No petals, leaves, body, sweat, blush, eyebrows, text or additional accessories. Transparent background with actual alpha, no checkerboard painted in. One isolated oval face filling the canvas, no white border, suitable for scaling back to the original 58 by 43 pixel bone texture. Do not make it glossy, realistic, more detailed, more saturated, or a sticker.

## Blue water drop

- Mode: built-in image generation tool, stylized-concept.
- Final asset: `addons/art/awkward-blue-drop.png` (48 × 72 RGBA).
- Preparation: `scripts/prepare-awkward-drop.mjs` trims transparent padding and downsamples while retaining the generated silhouette, colors and alpha.
- Runtime: one 24 × 36 local-unit water drop beside the forehead, attached to the face bone. No particle stream, cycling, drifting or fading. Appears only during embarrassment (plus card/almanac previews). Normal sunflower is untouched.

### Final prompt

Use case: stylized-concept.
Asset type: one small transparent PNG game sprite, an embarrassment sweat DROP accessory for an original Sunflower-style character in a 2009 Plants vs Zombies fan game.
Primary request: EXACTLY ONE LARGE BLUE WATER DROP. A clear pointed top and a plump rounded bottom, classic comically oversized embarrassment water droplet. Upright, slightly hand-drawn asymmetry, width about 60% of its height. One connected silhouette, not droplets or specks.
Style/medium: simple old-fashioned 2D cartoon game art like the original 2009 Plants vs Zombies, slightly uneven thick dark blue outline, muted medium sky blue fill, one restrained darker blue side shade and one broad pale blue/white highlight. Readable when reduced to 18 by 28 pixels. Flat painted colors, no real-world texture, no elaborate detailing, no photorealism, no 3D, no shiny glass rendering, no neon glow.
Composition: center the single complete drop with transparent padding; no character, face, plant, text, label, shadow, splash, trail, smaller satellite drops, scene, border or watermark. Genuine alpha transparency, not a checkerboard painted in the image. The isolated blue drop itself is the ONLY visible object.
