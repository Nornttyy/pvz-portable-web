# 缩头乌葵

- Replaces native sunflower in slot 1, retaining logical ID 520, 50-sun price and 3-second card cooldown. Unlocks with the original sunflower. Native slot 53/string key remains migration-only, not a separate selectable card.
- Cards and almanac show the normal standing sunflower. Only a planted sunflower near a hostile zombie uses the tucked pose. Adventure, imitater and the unified sandbox catalogue use the replacement in place, without an extra “new card” category.
- Nearby hostile ground zombies in the same lane trigger tucking, based on their native attack rectangles: enter within 60 native pixels of the plant bounds, recover beyond 84. This prevents repeated switching at the boundary.
- The whole head (face, petals and blink) uses a shared 0.8 scale and 24-pixel downward shift; stalk tracks retract to 0.35 height. Leaves and roots keep their native pose. No new bitmap, generated face, sweat overlay or per-part turn is used.
- Chewing and vault targeting skip a tucked flower. Other plants remain targetable; crushing and collateral damage are not globally disabled.
- Native 25-sun production/countdown is unchanged while tucked. No crowd gaze or production penalty remains.
- Optional ten-int saves use version 3. Version 1 embarrassment saves and old native slot 53 plants/seed-bank entries migrate in place without changing health, native sun countdown or campaign progress.
- QA: `scripts/qa-tucking-sunflower.mjs` covers real native zombie pass-through, backline eating, recovery/repeated tucking, adventure card, hidden save/resume, phone layout and almanac.
