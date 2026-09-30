#include "SeedChooserLayout.h"
#include "AlmanacPlantLayout.h"
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
        if (expanded) {cards.push_back(Card(SEED_LEFTPEATER, true, upgrades));cards.push_back(Card(SEED_AWKWARD_SUNFLOWER, true, upgrades));}
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
    for(bool expanded:{false,true}){
        std::vector<AlmanacPlantLayout::Box> cards;
        for(int seed=0;seed<48;++seed)cards.push_back(AlmanacPlantLayout::Card(seed,expanded));
        if(expanded){cards.push_back(AlmanacPlantLayout::Card(53,true));cards.push_back(AlmanacPlantLayout::Card(52,true));}
        for(unsigned i=0;i<cards.size();++i){const auto a=cards[i];assert(a.x>=26&&a.x+a.w<=442&&a.y>=92&&a.y+a.h<=552);
            for(unsigned j=0;j<i;++j){const auto b=cards[j];assert(!(a.x<b.x+b.w&&b.x<a.x+a.w&&a.y<b.y+b.h&&b.y<a.y+a.h));}}
        if(expanded){const auto extra=cards.back();assert(extra.y==cards.front().y&&extra.x==cards[7].x+46);}
    }
    static_assert(SEED_AWKWARD_SUNFLOWER==53&&NUM_SEED_TYPES==54&&SEED_BEGHOULED_BUTTON_SHUFFLE==54&&SEED_ZOMBIE_NORMAL==60);
    std::cout << "Native chooser: 50 full-size cards fit, unique slots, legacy layout unchanged; almanac extra card inside grid\n";
}
