# Native-derived plant repair

Mode: built-in `image_gen` editing, not CLI generation.

- Edit target: [unchanged native sprite contact sheet](native-redraw-reference.png).
- Final atlases: [open eyes](generated/native-redraw.png), [blink](generated/native-blink.png).
- Prompts: [color variants](prompts-native-redraw.json), [blink edit](prompt-native-blink.txt).
- Runtime assets: `parts/`, enumerated in `parts.json`. Native canvases and bone transforms are preserved. Mechanical extraction is reproducible with `scripts/register-native-redraw.swift`.
- Original Gatling housing, barrels, overlay, helmet and painter order are reused. Gatling variants have their own native-derived heads, not generic pea heads. The projectile origin follows the front barrel, not the rear housing.
- Tiny/heavy/scatter/seeker/acid mouths reuse the original pea lip unchanged. Their existing generated identity features remain.

[Plant pose proof](plant-poses-proof.png): all 18 custom plants, in ID order, six columns. Each cell shows idle, shooting and blink. This is an offline composition from original `.reanim` transforms, **not a live browser screenshot**; it does not verify runtime attachments, pointer handling or GPU output. CPU tests cover actual production skin selection, layer preservation and projectile coordinates. The native WASM build separately checks real engine integration.

Sandbox UI: 1024 × 600 native canvas; original 800 × 600 board moved right by 224 pixels. The persistent 264-pixel catalog overlaps only the original 40-pixel house margin. All 33 zombies fit in five columns; plants use native/custom tabs and native pagination. Only settings are modal. Adventure restores the 800 × 600 canvas.

No new claims of ownership or permission are made for native source artwork. Existing project attribution remains unchanged.
