# 双----------双发射手

- Logical ID 521 replaces Repeater in native slot 7; same 200-sun cost, unlock gate, native rig, portrait and ordinary pea assets. Uses the existing replacement-card 3-second refresh.
- Once a target is present, commits to 50 peas, one every 2 engine ticks (100 Hz), then rests 150 ticks. No scatter, knockback, manual trigger or extra status effect. Initial readiness is 50 ticks; no new volley without a target.
- Native recoil restarts every ten peas instead of every shot. Ordinary native movement, collision and splat remain intact.
- Projectile style 297 marks weak peas only: 20 native damage becomes 1; native torchwood fire conversion becomes 2. Other plants' peas keep their damage. The tag and partial volley survive optional saves without gaining extra shots; a full projectile pool stalls rather than discards shots.
- Tests: actual production C++ combat/save tests, native-slot/catalogue tests and `scripts/qa-tucking-sunflower.mjs` (WASM UI replacement, exact 50 peas, adventure cards, standing portrait and almanac).
