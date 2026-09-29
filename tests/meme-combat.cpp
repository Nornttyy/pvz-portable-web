#include "Engine.h"
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "SandboxArt.h"
#include "SandboxMemeRules.h"
#include <cassert>
#include <iostream>
LawnApp app;LawnApp* gLawnApp=&app;bool gSandboxEnabled=true;
namespace SandboxArt {
Sexy::Image* Image(const char*,const char*){return nullptr;}
Sexy::Image* NativeImage(const char*){return nullptr;}
Sexy::Image* WarmNative(const char*,int){return nullptr;}
void DrawFit(Sexy::Graphics*,Sexy::MemoryImage*,int,int,int,int,float){}
void Sprite(Sexy::Graphics*,const char*,float,float,float,float,float,int){}
void Link(Sexy::Graphics*,float,float,float,float,int,int){}
bool TrackPoint(Reanimation*,const char*,float,float,float,float,float&,float&){return false;}
}
struct World:Board {
 World(){SandboxPlants::Reset();SandboxZombies::Reset();}
 Plant* add(int id,int row=2){auto* p=plant(1,row);p->mSeedType=static_cast<SeedType>(SandboxPlants::Base(id));if(id==122)p->mPlantHealth=p->mPlantMaxHealth=4000;SandboxPlants::Assign(p,id);return p;}
 Zombie* enemy(float x=500,int row=2,ZombieType type=ZOMBIE_NORMAL){auto* z=AddZombieInRow(type,row,-1);z->mPosX=x;return z;}
 void step(int count=1){while(count--){SandboxPlants::Tick(this);SandboxZombies::Tick(this);if(!mPaused)++mMainCounter;}}
};
int main(){
 using namespace SandboxMemeRules;
 {World w;auto* p=w.add(Pea);w.step(2000);assert(w.mProjectiles.mSize==0&&SandboxPlants::HeatData(p,1)==0);
  w.enemy();w.step(625);assert(SandboxPlants::HeatData(p,0)==Bursting&&w.mProjectiles.mSize==5);
  w.step(179);assert(w.mProjectiles.mSize==14);w.step();assert(SandboxPlants::HeatData(p,0)==Recovering);
  const int count=w.mProjectiles.mSize;w.step(RecoveryTime);assert(count==w.mProjectiles.mSize&&SandboxPlants::HeatData(p,0)==Warming);
  w.step(45);assert(w.mProjectiles.mSize==count+1);
  for(auto* s:w.mProjectiles){assert(!SandboxPlants::UsesCustomShotArt(s));assert(!SandboxPlants::Impact(s,w.mZombies.values[0]));assert(SandboxPlants::ShotRadius(s)==12);}
 }
 {World w;auto* p=w.add(Pea);auto* z=w.enemy();w.step(200);const int heat=SandboxPlants::HeatData(p,1);assert(heat>0);
  z->mDead=true;const int count=w.mProjectiles.mSize;w.step(1000);assert(SandboxPlants::HeatData(p,1)==0&&w.mProjectiles.mSize==count);}
 {World w;auto* p=w.add(Pea);w.enemy();w.step(100);const int heat=SandboxPlants::HeatData(p,1),shots=w.mProjectiles.mSize;
  w.mPaused=true;w.step(3000);assert(SandboxPlants::HeatData(p,1)==heat&&shots==w.mProjectiles.mSize);
  w.mPaused=false;p->mIsAsleep=true;w.step(3000);assert(SandboxPlants::HeatData(p,1)==heat&&shots==w.mProjectiles.mSize);
  p->mIsAsleep=false;p->mSquished=true;w.step(3000);assert(shots==w.mProjectiles.mSize);
  p->mSquished=false;p->airborne=true;w.step(3000);assert(shots==w.mProjectiles.mSize);}
 {World w;auto* p=w.add(Pea);w.enemy();w.mProjectiles.mSize=w.mProjectiles.mMaxSize-8;w.step(5000);
  assert(w.ownedShots.empty()&&SandboxPlants::HeatData(p,1)==0);w.mProjectiles.mSize=0;w.step();assert(w.ownedShots.size()==1);}
 {World w;auto* p=w.add(Sunflower);w.step(600+SunInterval*2);assert(w.mCoins.mSize==3&&SandboxPlants::HeatData(p,0)==Bursting);
  w.step(BurstTime);assert(w.mCoins.mSize==6&&SandboxPlants::HeatData(p,0)==Recovering);
  w.step(SunRest);assert(w.mCoins.mSize==6&&SandboxPlants::HeatData(p,0)==Warming);
  w.step(SunInterval);assert(w.mCoins.mSize==7);}
 {World w;auto* p=w.add(Sunflower);w.mCoins.mSize=w.mCoins.mMaxSize-8;w.step(5000);
  assert(SandboxPlants::HeatData(p,1)==0);w.mCoins.mSize=0;w.step();assert(w.mCoins.mSize==1);}
 {World w;auto* p=w.add(Wallnut);auto* near=w.enemy(100),*far=w.enemy(500),*other=w.enemy(100,1),*heavy=w.enemy(110,2,ZOMBIE_GARGANTUAR),*friendly=w.enemy(100);friendly->mMindControlled=true;
  p->mPlantHealth-=200;w.step();assert(SandboxPlants::HeatData(p,1)==600&&near->mPosX==100);
  p->mPlantHealth-=134;w.step();assert(near->mPosX==165&&far->mPosX==500&&other->mPosX==100&&heavy->mPosX==110&&friendly->mPosX==100);
  assert(SandboxPlants::HeatData(p,0)==Recovering&&p->mPlantHealth==3666);
  p->mPlantHealth-=500;w.step(NutRest);assert(near->mPosX==165);w.step();assert(SandboxPlants::HeatData(p,1)==0);
  p->mPlantHealth-=334;w.step();assert(near->mPosX==230);}
 {World w;auto* p=w.add(Wallnut);p->mPlantHealth=1;SandboxPlants::Assign(p,Wallnut);w.step();assert(SandboxPlants::HeatData(p,1)==0&&p->mPlantHealth==1);
  p->mPlantHealth=0;w.enemy(100);w.step();assert(SandboxPlants::HeatData(p,1)==0);}
 {World w;auto* a=w.add(Pea),*b=w.add(Pea,1);w.enemy();w.step(645);assert(SandboxPlants::HeatData(a,1)==MaxHeat&&SandboxPlants::HeatData(b,1)==0);
  SandboxPlants::Forget(a);assert(SandboxPlants::HeatData(a,0)==-1);SandboxPlants::Reset();assert(SandboxPlants::HeatData(b,0)==-1);}
 {const uint32_t white=0xffffffff,black=0xff111111,green=0x8016c54c,brown=0xffb88742;
  assert(WarmPixel(white,24)==white&&WarmPixel(black,24)==black&&WarmPixel(0x0000ff00,24)==0x0000ff00);
  assert(WarmPixel(green,0)==green&&(WarmPixel(green,24)>>24)==0x80);
  assert(((WarmPixel(green,24)>>16)&255)>((WarmPixel(green,24)>>8)&255));assert(WarmPixel(brown,24)!=brown);
  for(int level=0;level<=24;++level)assert((WarmPixel(green,level)>>24)==0x80);}
 std::cout<<"Meme powers: production phase cycles, roles, pause, caps, cleanup and recolour passed\n";
}
