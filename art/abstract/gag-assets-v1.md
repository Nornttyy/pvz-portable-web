# Gag animation assets, version 1

The two PNG source files were made with the built-in image-generation tool, using the original Squash body and zombie hand/bucket as visual references. Native parts are mechanically alpha-cropped and scaled by `scripts/register-gag-art.swift`; no original texture is overwritten.

## Prompts

Runtime images: `addons/art/squash-exercise.png` (86×96), `addons/art/bucket-glove.png` (24×26). Generated originals: `art/abstract/squash-exercise-source-v1.png`, `art/abstract/bucket-glove-source-v1.png`.

### Squash body (built-in image generation, reference: Squash_body.png)

Edit this supplied low-resolution 2009 cartoon game squash BODY SPRITE only, for a native skeletal-animation image replacement. Preserve its exact squat lumpy vegetable silhouette, olive-green muted palette, thick irregular dark outlines, angry face and eye locations. Add a faded dusty-purple elastic exercise sweatband wrapping around the upper forehead ABOVE the eyes, with two short floppy tied tails sticking from the LEFT edge, and make the lower belly have two broad soft accordion folds so it looks like an absurd exhausted exercise enthusiast that squashes flat and bounces back. Keep all original face/joint positions and head stem connection at top. Sparse simple broad shading, deliberately modest old raster-game detail, no realism, no grain or texture, no gloss, no text or symbols, no extra limbs, no floor, no shadow outside sprite. Single isolated complete sprite centered on genuinely TRANSPARENT background, no checkerboard rendered, no sprite sheet, no additional views. Preserve transparent margins, fully visible tails. This is a usable animation body part, not a concept scene.

### Glove (built-in image generation, references: Zombie_outerarm_hand.png, Zombie_bucket1.png)

Create ONE game skeletal-animation cutout: a comically oversized worn dull burgundy boxing glove on a very short olive-gray zombie wrist. Reference1 gives wrist direction and original game palette/outline, reference2 gives original chunky simple 2009 cartoon game shading. Fist hangs pointing DOWN, wrist opening at the TOP center, thumb bulge on the RIGHT, so this can replace a dangling zombie hand and swing with the existing forearm. Glove rounded with exactly one simple seam and two broad muted shades, thin almost-black irregular outline. No text, no logo, no stars, no extra hands, no arm beyond short wrist. Do not include bucket or other objects. Single isolated fully visible part on genuine transparent alpha background, no shadow, no checkerboard baked in. Deliberately simple readable at 28x36 pixels; no realistic leather texture, no grain, no bright saturated red, no detailed highlights. This is a usable replacement sprite part, not a character illustration.

## Audio provenance

`rage-scream-source.mp3`: joseppujol, “Horror scream (high-pitched voice)”, 2014. Public preview from https://cdn.freesound.org/previews/221/221547_4096172-hq.mp3 . Source and CC0 dedication: https://freesound.org/people/joseppujol/sounds/221547/ . License: https://creativecommons.org/publicdomain/zero/1.0/ . This is a licensed human recording, not a cloned or generated voice.

`addons/audio/rage-scream.wav`: converted to mono PCM, leading/trailing silence removed, vocal resampled to three seconds (raising pitch), shallow syllabic tremolo, normalized below clipping, 25/90 ms edge fades. Reproduce: `afconvert -f WAVE -d LEI16 art/abstract/rage-scream-source.mp3 /tmp/pvz-rage-source.wav`, then `node scripts/make-rage-audio.mjs /tmp/pvz-rage-source.wav`.
