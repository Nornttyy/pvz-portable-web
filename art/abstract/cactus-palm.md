# 仙人的掌：手掌分件

Generated with the built-in image_gen tool (imagegen skill). Original cactus
body, eyes, arms, lips, idle/rise/recoil animations remain native. Only the
horizontal palm is new artwork. No new import/upload step at runtime: its
alpha pixels are embedded in the shipped game binary.

- Source: `cactus-palm-source-v1.png`
- Runtime raster: `cactus-palm-native-v1.png` (64×44, genuine alpha)
- Reproducible alpha trim/resample: `scripts/register-cactus-palm.swift`
- Embedded pixels: `src/CactusPalmPixels.h`
- Registration: opaque wrist pixel `(6,29)` sits at `(5,13.5)` inside the native
  `Cactus_lips` opening. Draw after the native body/tube/rim, so its dark fill
  cannot cut off the wrist. No replacement/deletion of `Cactus_mouth`.
  Preview and projectile use the same transform, including raised/roof poses.

## Final prompt

Use case: stylized-concept. Asset type: one isolated transparent game sprite attachment and projectile. Primary request: a single small cartoon open human palm, held horizontally, short wrist stub on the LEFT, four fingers pointing RIGHT, thumb above, palm facing the viewer in a slight three-quarter view, as if pushing to the right. Style: compatible with the original 2009 Plants vs Zombies PC raster art: slightly wonky hand-painted cartoon, chunky dark brown outline, muted warm cream and tan, only two or three gentle shaded tones. No photorealism or realistic texture. Readable silhouette at 48 by 32 pixels. Composition: exactly ONE hand centered with transparent padding all around, fills most of a landscape canvas; no arm, body, cactus, card, lettering, effects, ground, shadow, scene or extras. Genuinely transparent background, not a checkerboard drawing. Five digits total, no missing or extra fingers. Broad friendly palm, fingers together with subtly uneven tips, thumb distinct. Flat simple game art, not glossy, not 3D, no skin pores, no ornate detail.
