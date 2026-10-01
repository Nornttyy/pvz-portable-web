# 加特林射手

- Replaces native Gatling Pea (slot 40 / `GATLING_PEA`) with logical ID 522. Keeps the original rig, card portrait, 250-sun cost, purchase gate and adventure Repeater upgrade requirement. Sandbox retains its existing free upgrade placement. Both use the same replacement; no extra card/category.
- One straight native pea every 10 simulation ticks (0.1 seconds), 15 damage per hit. Only actual shots add heat. 120 shots (12 seconds of sustained fire) trigger overheat; then no firing for exactly 350 ticks (3.5 seconds), followed by automatic recovery. No passive damage, manual activation, recoil knockback or new debuff.
- No target or a full projectile pool cannot add heat. Existing heat waits during a targeting gap; only a completed overheat cycle resets it. Each plant has its own state; pause, sleep and airborne states suspend combat ticks.
- Small orange heat bar; during cooldown it drains in muted blue and uses native smoke art. No added dialogue or generated replacement textures. Native recoil is restarted once per 10 shots, not every projectile.
- Projectile style 298 retains per-shot damage through save/restore and native torchwood conversion (30 fire damage). Does not affect the 1-damage Repeater peas, their 60% visual scale, rage peas or ordinary projectiles.
- Optional saves preserve heat, attack interval and remaining cooling time. Existing native Gatling Peas acquire the new mechanics on adventure restore without changing HP, placement or campaign progress.
