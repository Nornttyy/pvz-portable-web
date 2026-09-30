# Two-character roster

Requested scope: remove every original plant and zombie except 红温豌豆 and 反咬坚果.

- Active character IDs: 500 (native peashooter slot), 501 (native wall-nut slot).
- Adventure still uses the original progression and card identities. All other native plants, prices, cooldowns, previews and zombie behaviors are restored. The retired self-thrower no longer appears in the chooser or almanac.
- Sandbox accepts the 48 native plant types plus the two retained characters. The default hotbar has no retired IDs. The native zombie roster is unchanged.
- Old formation IDs 502–518 convert to their native bases; the self-thrower becomes a peashooter. Legacy infusion IDs remain migration-only and cannot be planted or restored as powered characters.
- Adventure migration resets retired attack timers, releases old zombie hold/pin/carry phases, restores the original plant anchors when necessary, and retains HP, armor and player progress.
- Red pea's 300 rage, 3-second 50-pea release, 10% ordinary hit decision, random-amplitude up/down flight, slower burst speed, gradual redness, automatic release, 3-second recovery and vocal are unchanged.
- Walnut's 3-second, 80-damage counterattack, forward bump, angry eyebrows, native damage stages and no-knockback behavior are unchanged.
- General sun collection, 25-value sun appearance, faster production, stacking, continuous zombie placement, mobile sizing and resource loading are retained.
- Retired accessory source PNGs remain recoverable in the repository but are no longer embedded into the game. No new artwork or audio was generated.

Verification: production C++ roster/combat/visual/migration tests, complete Pages suite, isolated real-WASM rage regression and roster/adventure browser QA.
