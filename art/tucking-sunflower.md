# 缩头乌葵

- Replaces 尬葵, retaining logical ID 520, native slot 53, 50-sun price, adventure 1-6 unlock and 3-second card cooldown. The legacy native enum/string key is deliberately retained for compatibility.
- Nearby hostile ground zombies in the same lane trigger tucking, based on their native attack rectangles: enter within 60 native pixels of the plant bounds, recover beyond 84. This prevents repeated switching at the boundary.
- The whole head (face, petals and blink) uses a shared 0.8 scale and 24-pixel downward shift; stalk tracks retract to 0.35 height. Leaves and roots keep their native pose. No new bitmap, generated face, sweat overlay or per-part turn is used.
- Chewing and vault targeting skip a tucked flower. Other plants remain targetable; crushing and collateral damage are not globally disabled.
- Native 25-sun production/countdown is unchanged while tucked. No crowd gaze or production penalty remains.
- Optional ten-int saves use version 3. Version 1 embarrassment saves migrate in place without changing health, native sun countdown or campaign progress.
- QA: `scripts/qa-tucking-sunflower.mjs` covers real native zombie pass-through, backline eating, recovery/repeated tucking, adventure card, hidden save/resume, phone layout and almanac.
