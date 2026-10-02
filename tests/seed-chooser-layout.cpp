#include "SeedChooserLayout.h"
#include "AlmanacPlantLayout.h"
#include "MemeAdventure.h"
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
        if (expanded) {cards.push_back(Card(SEED_LEFTPEATER, true, upgrades));cards.push_back(Card(SEED_SMALL_NUT, true, upgrades));cards.push_back(Card(SEED_SPROUT,true,upgrades));cards.push_back(Card(SEED_EXPLODE_O_NUT,true,upgrades));}
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
            assert(Slot(SEED_SMALL_NUT,true)==9);
            assert(Card(SEED_SMALL_NUT,true,upgrades).x==22);
            assert(cards[8].x == 75 && cards[8].y > cards[0].y);
        }
    }
    for(bool expanded:{false,true})for(int page=0;page<(expanded?2:1);++page){
        std::vector<AlmanacPlantLayout::Box> cards;
        for(int seed=0;seed<54;++seed)if(seed!=48&&AlmanacPlantLayout::Visible(seed,expanded,page))cards.push_back(AlmanacPlantLayout::Card(seed,expanded));
        assert(cards.size()==(page==0?48:4));assert(AlmanacPlantLayout::Scale(expanded)==1.f);
        for(unsigned i=0;i<cards.size();++i){const auto a=cards[i];assert(a.x>=26&&a.x+a.w<=442&&a.y>=92&&a.y+a.h<=552);
            assert(page==1||(a.x==26+i%8*52&&a.y==92+i/8*78&&a.w==50&&a.h==70)); // Page one exactly matches the printed slots.
            for(unsigned j=0;j<i;++j){const auto b=cards[j];assert(!(a.x<b.x+b.w&&b.x<a.x+a.w&&a.y<b.y+b.h&&b.y<a.y+a.h));}}
        for(int seed=0;seed<48;++seed)assert(AlmanacPlantLayout::Page(seed,expanded)==0);
        for(int i=0;i<4;++i){const int seed=AlmanacPlantLayout::Extras[i];assert(AlmanacPlantLayout::Page(seed,expanded)==(expanded?1:-1));if(expanded){const auto b=AlmanacPlantLayout::Card(seed,true);assert(b.x==26+i*52&&b.y==92&&b.w==50&&b.h==70);}}
        assert(AlmanacPlantLayout::Page(50,expanded)==-1&&AlmanacPlantLayout::Visible(48,expanded,0)&&!AlmanacPlantLayout::Visible(48,expanded,1));
    }
    for(int page=0;page<2;++page)for(int y=0;y<600;++y)for(int x=0;x<800;++x){
        const int turn=AlmanacPlantLayout::TurnAt(x,y,page,2);
        assert(turn==(page==0&&AlmanacPlantLayout::Next.Contains(x,y)?1:page==1&&AlmanacPlantLayout::Previous.Contains(x,y)?-1:0));
    }
    assert(AlmanacPlantLayout::TurnAt(408,580,0,1)==0);
    static_assert(SEED_SMALL_NUT==53&&NUM_SEED_TYPES==54&&SEED_BEGHOULED_BUTTON_SHUFFLE==54&&SEED_ZOMBIE_NORMAL==60);
    static_assert(MemeAdventure::LegacyShooterSlot(51800)&&!MemeAdventure::LegacyShooterSlot(51900)&&!MemeAdventure::LegacyShooterSlot(52300));
    static_assert(MemeAdventure::LegacySunflowerSlot(51900)&&!MemeAdventure::LegacySunflowerSlot(52300));
    std::cout << "Native chooser: 52 full-size cards fit, unique slots, legacy layout unchanged; almanac extras inside grid\n";
}
