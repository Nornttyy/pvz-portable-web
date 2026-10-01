# 智斗僵尸表情与刻痕下颚

Created with the built-in imagegen skill/tool, not the fallback API. Only the head was redrawn; the existing native body, cone and pool ring are reused. IDs 216 and 217 only.

## Saved assets

- `clever-head-source-v1.png`: generated neutral/scheming complete head, transparent alpha.
- `clever-jaw-source-v1.png`: generated action head, angular jaw with bold crease/hatch marks.
- `clever-head-native-v1.png` and `clever-jaw-native-v1.png`: game-size 55×62 sprites.
- `../../src/CleverHeadPixels.h`: embedded ARGB data, consumed by SandboxArt::CleverHead.

## Registration

The two 1254×1254 generated canvases share one crop (367,352,548,618), scaled uniformly to 53 pixels of ink plus clear padding. No independent trimming, procedural face drawing, palette replacement or fake transparency. Both use the same native head1 bone and local (-1,-5) offset. Head2/jaw, tongue and duplicate hair are hidden only within the draw scope; original cone/body transforms and death visibility are preserved. Normal expression in cards and almanac; marked jaw for backflip/lane change and briefly after stealing.

Regenerate embedded data with `swift scripts/register-clever-head.swift <repo> <neutral-source.png> <jaw-source.png>`. Existing saved sources are reused non-destructively.

## Final prompt set

### Neutral (references: native reanim/Zombie_head.png and Zombie_jaw.png)

Use case: precise-object-edit. Asset type: production transparent head sprite for an existing low-resolution 2D zombie skeletal rig. Image 1 is the native upper head to preserve, image 2 is its native lower jaw, same character; combine them into ONE complete isolated head and change only the expression. Preserve the original classic Plants vs Zombies 2009 comic style, exact muted olive gray-green skin, dark uneven thick contour, tiny shading patches, sparse hair, left-facing three-quarter angle, off-white asymmetric eyeballs, stubby nose, crooked few teeth. New expression: a comically calculating, smug, very self-confident mastermind (智斗): narrow half-lidded eyes with visible pupils looking left, one eyebrow lifted, a tiny scheming crooked smirk. No glasses, no sunglasses, no props, no hands, no body, no hat, no beard. Ordinary rounded lower jaw in this neutral version, not a giant chin yet. Compact head only; bottom ends at its original short neck stub; preserve the skull proportions, eye and nose positions from Image 1 so it can fit the original cone hat and neck. Do not make it human, realistic, muscular or anime. Very simple flat cartoon raster artwork suitable for downsampling to approximately 53 by 58 pixels, no fine detailing or textures, no glossy lighting. One sprite centered in square canvas, genuinely transparent background with alpha, ample fully clear padding. No text, no grid, no shadow, no watermark. Match the supplied original head style closely.

### Angular jaw edit (target: generated neutral head)

Use case: precise-object-edit. Image 1 is the edit target: a production zombie head sprite. Change ONLY its lower cheek, jawline and chin to an exaggerated sharply angular projecting meme jawline, comically smug mastermind / 智斗 mewing expression. Maintain exactly the same forehead, skull, eyes, pupils, eyebrows, hair, nose, upper mouth, placement, scale, facing and muted olive colors. The upper 75 percent of this head must stay unchanged so a native cone hat will stay perfectly registered. Strong diagonal cheek-to-chin contour and angular protruding chin, visibly more defined than the rounded original, but keep it cartoon and simple, not realistic, not muscular human. Keep a crooked confident smirk and a few teeth. Same classic Plants vs Zombies 2009 flat low-resolution cartoon style, thick irregular dark outlines, no added textures, no glasses, no props, no body. ONE complete isolated head, same square canvas and same placement as input, real transparent alpha background, no checkerboard, no text or watermark. This will swap with the input sprite during backflips, lane changes and stealing, so preserve skull alignment, extend lower jaw only.

### Visible crease marks edit (target: angular jaw variant)

Use case: precise-object-edit. Image 1 is the edit target, an existing transparent cartoon zombie head. Make ONE very small precise addition: add clearly visible dark carved-looking contour crease marks along its exaggerated angular lower cheek and jawline, as comical 智斗 / mewing jaw-definition marks. A bold slanted inner jaw contour plus TWO short broad hatch/crease marks crossing or beside it; marks must remain readable downsampled to a 53-pixel-wide game sprite, no tiny detail. No blood, no realistic scars or wounds, no beard. Keep EVERYTHING ELSE unchanged: same eye expression, colors, skull and forehead silhouette, hair, nose, mouth, protruding chin shape, transparent canvas, exact position and size. Preserve simple muted classic Plants vs Zombies 2009 cartoon style with thick dark lines and flat shading. No glasses, no props, no text, no watermark, no body. Real transparent alpha background. This is an action-only face variant, not an illustration.

