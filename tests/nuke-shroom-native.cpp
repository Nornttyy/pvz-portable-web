// Compile the real detonation and pulse code against bounded engine spies.
#include "NukeShroomRules.h"
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
namespace Sexy {enum {SOUND_DOOMSHROOM};struct Color {int r,g,b,a;Color(int r,int g,int b,int a=255):r(r),g(g),b(b),a(a){}};}
enum {GRIDITEM_CRATER,GRIDITEM_LADDER,GRIDITEM_GRAVESTONE,SEED_COBCANNON=47,RENDER_LAYER_TOP,PARTICLE_DOOM};
enum GridItemState {STATE_NORMAL};
class Board;
struct Plant {Board* mBoard;int type=526,mPlantCol=4,mRow=2,mX=400,mY=250,mSeedType=15;bool mDead=false;void Die(){mDead=true;}};
namespace MemeCharacters {int Type(Plant* p){return p->type;}}
struct Zombie {bool mDead=false,mMindControlled=false,onBoard=true,dying=false,vulnerable=true;int health=12000,hits=0;
 bool IsOnBoard(){return onBoard;}bool IsDeadOrDying(){return dying;}bool EffectedByDamage(int flags){assert(flags==127);return vulnerable;}
 void ApplyBurn(){++hits;health-=1800;if(health<=0)mDead=true;}};
struct GridItem {Board* mBoard=nullptr;bool mDead=false;int mGridItemType=GRIDITEM_CRATER,mGridX=0,mGridY=0,mGridItemCounter=0,mSunCount=0,mTransparentCounter=0;
 GridItemState mGridItemState=STATE_NORMAL;float mPosX=0,mPosY=0;void GridItemDie(){mDead=true;}};
struct Particle {std::map<std::string,Sexy::Color> colors;void OverrideColor(const char* name,Sexy::Color c){colors.insert_or_assign(name?name:"all",c);}};
struct App {int sounds=0;std::vector<Particle> particles;void PlaySample(int){++sounds;}Particle* AddPvzpParticle(float,float,int,int){particles.emplace_back();return &particles.back();}};
struct Items {std::vector<std::unique_ptr<GridItem>> owned;std::vector<GridItem*> entries;int mSize=0,mMaxSize=1024;
 auto begin(){return entries.begin();}auto end(){return entries.end();}GridItem* add(Board* b,int col,int row){owned.push_back(std::make_unique<GridItem>());auto* p=owned.back().get();p->mBoard=b;p->mGridX=col;p->mGridY=row;entries.push_back(p);++mSize;return p;}};
class Board {public:App app;App* mApp=&app;bool pool=false,mPaused=false;int shakes=0;Items mGridItems;std::vector<Plant*> mPlants;std::vector<Zombie*> mZombies;
 bool StageHasPool(){return pool;}GridItem* AddACrater(int col,int row){return mGridItems.add(this,col,row);}void ShakeBoard(int,int){++shakes;}};
#include "nuke-production.inc"
void step(Board& b,int count=1){while(count--)for(auto* item:b.mGridItems)NukeShroom::UpdateCrater(item);}
int main(){
 using namespace NukeShroomRules;
 static_assert(Size==3&&Cost==250&&Recharge==3000&&Pulses==5&&CraterLife==18000);
 for(int rows:{5,6})for(int r=0;r<rows;++r)for(int c=0;c<9;++c){
  Board b;b.pool=rows==6;Plant nuke{&b};nuke.mPlantCol=c;nuke.mRow=r;
  const auto area=Footprint(c,r,rows);assert(area.Contains(c,r));
  std::vector<Plant> field;field.reserve(rows*9*3);
  for(int y=0;y<rows;++y)for(int x=0;x<9;++x)for(int layer:{16,3,30}){field.push_back({&b});auto& p=field.back();p.mPlantCol=x;p.mRow=y;p.type=0;p.mSeedType=layer;b.mPlants.push_back(&p);}
  b.mPlants.push_back(&nuke);auto* grave=b.AddACrater(c,r);grave->mGridItemType=GRIDITEM_GRAVESTONE;
  Zombie farLeft,farRight,preview,friendly,corpse,immune;preview.onBoard=false;friendly.mMindControlled=true;corpse.dying=true;immune.vulnerable=false;
  b.mZombies={&farLeft,&farRight,&preview,&friendly,&corpse,&immune};
  auto* ladder=b.AddACrater(8,rows-1);ladder->mGridItemType=GRIDITEM_LADDER;
  assert(NukeShroom::Detonate(&nuke)&&nuke.mDead&&grave->mDead&&ladder->mDead);
  std::set<std::pair<int,int>> holes;GridItem* primary=nullptr;
  for(auto* item:b.mGridItems)if(NukeShroom::IsCrater(item)){
   assert(item->mGridX>=0&&item->mGridX<9&&item->mGridY>=0&&item->mGridY<rows);
   assert(item->mGridItemCounter==18000&&area.Contains(item->mGridX,item->mGridY));holes.emplace(item->mGridX,item->mGridY);
   if(int(item->mGridItemState)==CraterMarker)primary=item;
  }
  assert(holes.size()==9&&primary&&primary->mSunCount==4&&primary->mTransparentCounter==80);
  for(auto& p:field)assert(p.mDead==area.Contains(p.mPlantCol,p.mRow));
  assert(farLeft.hits==1&&farRight.hits==1&&b.app.sounds==1);
  b.mPaused=true;step(b,200);assert(primary->mTransparentCounter==80&&b.app.sounds==1);b.mPaused=false;
  for(int tick=1;tick<=400;++tick){step(b);assert(b.app.sounds==std::min(5,1+tick/80));}
  step(b,1000);assert(b.app.sounds==5&&farLeft.hits==5&&farRight.health==3000);
  for(auto* z:{&preview,&friendly,&corpse,&immune})assert(z->hits==0);
  assert(primary->mSunCount==0&&primary->mTransparentCounter==0&&b.shakes==5);
  for(const auto& p:b.app.particles){const auto green=p.colors.at("all");assert(green.g>green.r&&green.g>green.b);assert(p.colors.at("DoomFlash").a==0&&p.colors.at("DoomWord").a==0);}
 }
 // Portable save uses these native fields, not the now-dead mushroom.
 {Board b;Plant plant{&b};b.mPlants={&plant};NukeShroom::Detonate(&plant);step(b,117);auto* primary=b.mGridItems.entries[0];
  assert(b.app.sounds==2&&primary->mSunCount==3&&primary->mTransparentCounter==43);
  Board resumed;for(auto* source:b.mGridItems){auto* item=resumed.AddACrater(source->mGridX,source->mGridY);*item=*source;item->mBoard=&resumed;}
  step(resumed,42);assert(resumed.app.sounds==0);step(resumed);assert(resumed.app.sounds==1);step(resumed,160);assert(resumed.app.sounds==3);step(resumed,1000);assert(resumed.app.sounds==3);
 }
 // Two simultaneous blast clocks remain independent; native craters are ignored.
 {Board b;Plant a{&b},c{&b};c.mPlantCol=8;c.mRow=4;b.mPlants={&a,&c};auto* native=b.AddACrater(0,0);native->mSunCount=99;native->mTransparentCounter=19;
  assert(NukeShroom::Detonate(&a));step(b,20);assert(NukeShroom::Detonate(&c));step(b,320);assert(b.app.sounds==10);
  assert(!NukeShroom::IsCrater(native)&&native->mSunCount==99&&native->mTransparentCounter==19);
 }
 {Board b;Plant nuke{&b},cannon{&b},outside{&b};cannon.mPlantCol=2;cannon.mSeedType=SEED_COBCANNON;outside.mPlantCol=1;outside.mSeedType=SEED_COBCANNON;
  b.mPlants={&nuke,&cannon,&outside};NukeShroom::Detonate(&nuke);assert(cannon.mDead&&!outside.mDead);
 }
 {Board b;Plant native{&b};native.type=0;assert(!NukeShroom::Detonate(&native)&&b.mGridItems.mSize==0);}
 // Fine energy veins keep the original alpha and dark outlines at every phase.
 int changes=0,unchanged=0,flow=0;
 for(int y=0;y<70;++y)for(int x=0;x<60;++x)for(int phase=0;phase<8;++phase){
  assert(EnergyPixel(0x00999999,x,y,phase)==0x00999999&&EnergyPixel(0xff171717,x,y,phase)==0xff171717);
  const auto p=EnergyPixel(0x80908070,x,y,phase);assert((p>>24)==0x80);
  if(p==0x80908070)++unchanged;else{++changes;assert(((p>>8)&255)>((p>>16)&255));}
  flow+=p!=EnergyPixel(0x80908070,x,y,(phase+1)%8);
 }
 assert(changes>0&&unchanged>changes&&flow>0&&FlashAlpha(80)==75&&FlashAlpha(35)==0&&FlashAlpha(0)==0);
 std::cout<<"Nuclear Shroom: nine cells at every board position, five timed pulses, native exclusions, saves and green art passed\n";
}
