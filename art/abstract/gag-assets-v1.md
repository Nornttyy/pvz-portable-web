# Gag animation assets, version 1

## Version 2: accessories only (current runtime)

The entire generated Squash body is RETIRED. Its source remains archived below, and its registered sprite was moved from `addons/art/squash-exercise.png` to `art/abstract/squash-exercise-retired-v1.png` (recoverable, no longer embedded). The original `Squash_body.png`, stem, animated face, eyes and blink are retained. At runtime a temporary cached composition adds only the generated accessory; native image definitions are never overwritten.

Built-in image generation (not CLI), final prompt:

> Use case: background-extraction / accessory-only game sprite. Image 1 is the ORIGINAL character whose body and face must NOT be redrawn. Image 2 is a reference ONLY for its muted dusty-purple exercise headband. Output ONLY that headband as one isolated usable transparent game accessory: a shallow curved purple front cloth strip, knot at left, two short floppy tails extending left. The cloth strip is thin and wide, slight upward slant to the right, seen from the same front three-quarter view as the references. Keep exactly the muted purple palette, simple dark irregular outline, 2 broad shade regions, modest old 2009 cartoon game detail. Genuine transparent alpha everywhere outside the fabric, including the whole area where the head would be. ABSOLUTELY NO vegetable, body, face, eyes, skin, stem, mannequin, outline of a head, floor, cast shadow, text, checkerboard or extra object. Do not generate an entire character. Accessory only, centered and fully visible, intended to be attached ABOVE the original eyes using the existing skeleton. No realistic cloth texture or tiny decorative detail.

Saved generation: `art/abstract/squash-headband-source-v2.png`. Runtime part: `addons/art/squash-headband.png`, 86×96 true-alpha canvas, 73×26 visible registration at (0,8). `scripts/register-gag-art.swift` performs only mechanical crop/resize/registration.

Current vocal: `art/abstract/rage-open-source-v2.mp3`, JohnsonBrandEditing, **human male scream 1**, https://freesound.org/people/JohnsonBrandEditing/sounds/243377/ ; CC0 https://creativecommons.org/publicdomain/zero/1.0/ . Public preview https://cdn.freesound.org/previews/243/243377_3229685-hq.mp3 . Not generated, cloned or celebrity imitation. Source is trimmed, WSOLA time-stretched with waveform-matched overlaps and pitched up six semitones, brightened gently, faded, normalized to peak 0.8. No artificial tremolo/vibrato, no voice stacking. Runtime 24 kHz mono PCM, exactly three seconds, native SFX volume 60%.

Reproduction: `afconvert -f WAVE -d LEI16 art/abstract/rage-open-source-v2.mp3 /tmp/pvz-rage-open-source.wav` and `node scripts/make-rage-audio.mjs /tmp/pvz-rage-open-source.wav`.

Everything below documents the retired first attempt, not the current Squash body or vocal.

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
