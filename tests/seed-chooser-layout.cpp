#include "SeedChooserLayout.h"
#include <cassert>
#include <iostream>
#include <vector>

int main() {
    using namespace SeedChooserLayout;
    for (bool expanded : {false, true}) for (bool upgrades : {false, true}) {
        std::vector<Position> cards;
        const int count = upgrades ? 48 : 40;
        const auto imitater = Imitater(expanded);
        assert(imitater.x >= 465 + ExtraWidth(expanded) - 1);
        // Include the socket's outer artwork, not only its clickable card.
        assert(imitater.x - 5 + 66 <= 560 || imitater.y - 12 + 93 <= 572);
        for (int seed = 0; seed < count; ++seed) cards.push_back(Card(seed, expanded, upgrades));
        if (expanded) cards.push_back(Card(SEED_LEFTPEATER, true, upgrades));
        for (unsigned i = 0; i < cards.size(); ++i) {
            const auto a = cards[i];
            assert(a.x >= 22 && a.x + 50 <= 465 + ExtraWidth(expanded) - 20);
            assert(a.y >= 123 && a.y + 70 <= 545);
            for (unsigned j = 0; j < i; ++j) {
                const auto b = cards[j];
                assert(!(a.x < b.x + 50 && b.x < a.x + 50 && a.y < b.y + 70 && b.y < a.y + 70));
            }
        }
        if (!expanded) for (int seed = 0; seed < count; ++seed) {
            assert(cards[seed].x == 22 + seed % 8 * 53);
            assert(cards[seed].y == (upgrades ? 123 : 128) + seed / 8 * (upgrades ? 70 : 73));
        }
        else {
            assert(Slot(SEED_LEFTPEATER, true) == 8);
            assert(Card(SEED_LEFTPEATER, true, upgrades).y == cards[0].y);
            assert(Card(SEED_LEFTPEATER, true, upgrades).x == cards[7].x + 53);
            assert(cards[8].x == 22 && cards[8].y > cards[0].y);
        }
    }
    std::cout << "Native chooser: 49 full-size cards fit, unique slots, legacy layout unchanged\n";
}
