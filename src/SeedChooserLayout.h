#pragma once
#include "ConstEnums.h"

// Keep native 50 x 70 cards and their spacing. The extra adventure plant
// occupies the ninth cell, rather than an Imitater-style external socket.
namespace SeedChooserLayout {
constexpr int ExtraWidth(bool expanded) { return expanded ? 53 : 0; }
constexpr int Columns(bool expanded) { return expanded ? 9 : 8; }
constexpr int Slot(int seed, bool expanded) {
    if (!expanded) return seed;
    if (seed == SEED_LEFTPEATER) return 8;
    if (seed == SEED_AWKWARD_SUNFLOWER) return 17;
    return seed >= 16 ? seed + 2 : seed >= SEED_PUFFSHROOM ? seed + 1 : seed;
}
struct Position { int x, y; };
constexpr Position Imitater(bool expanded) {
    // Align the original Imitater socket with the last grid row so that
    // widening the panel cannot cover the Almanac button at (560, 572).
    return {464 + ExtraWidth(expanded), expanded ? 473 : 515};
}
constexpr Position Card(int seed, bool expanded, bool upgrades) {
    const int slot = Slot(seed, expanded), columns = Columns(expanded);
    return {22 + slot % columns * 53,
            (upgrades ? 123 : 128) + slot / columns * (upgrades ? 70 : 73)};
}
}
