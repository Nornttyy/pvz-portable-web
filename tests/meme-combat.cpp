#include "Engine.h"
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "SandboxArt.h"
#include "SandboxMemeRules.h"
#include "MemeAdventureRules.h"
#include <cassert>
#include <iostream>
LawnApp app;LawnApp* gLawnApp=&app;bool gSandboxEnabled=true;
namespace SandboxArt {
Sexy::Image* Image(const char*,const char*){return nullptr;}
Sexy::Image* NativeImage(const char*){return nullptr;}
Sexy::Image* WarmNative(const char*,int){return nullptr;}
Sexy::Image* PowerNative(const char*,int,int){return nullptr;}
void DrawFit(Sexy::Graphics*,Sexy::MemoryImage*,int,int,int,int,float){}
void Sprite(Sexy::Graphics*,const char*,float,float,float,float,float,int){}
void Link(Sexy::Graphics*,float,float,float,float,int,int){}
bool TrackPoint(Reanimation*,const char*,float,float,float,float,float&,float&){return false;}
}
struct World:Board {
 World(){SandboxPlants::Reset();SandboxZombies::Reset();}
 Plant* add(int id,int row=2){auto* p=plant(1,row);p->mSeedType=static_cast<SeedType>(SandboxPlants::Base(id));if(SandboxMemeRules::Role(id)==SandboxMemeRules::Wallnut)p->mPlantHealth=p->mPlantMaxHealth=SandboxPlants::Base(id)==23?8000:4000;SandboxPlants::Assign(p,id);return p;}
 Zombie* enemy(float x=500,int row=2,ZombieType type=ZOMBIE_NORMAL){auto* z=AddZombieInRow(type,row,-1);z->mPosX=x;return z;}
 void step(int count=1){while(count--){SandboxPlants::Tick(this);SandboxZombies::Tick(this);if(!mPaused)++mMainCounter;}}
};
int main(){
 using namespace SandboxMemeRules;
 static_assert(MemeAdventureRules::Cooldown==300);
 for(int power=180;power<=182;++power)for(int base:Bases){const int id=Result(base,power);assert(IsResult(id)&&SandboxPlants::Find(id)&&BaseOf(id)==base&&PowerOf(id)==power);
  assert(!MemeAdventureRules::Unlocked(1,false,power,base));assert(MemeAdventureRules::Unlocked(50,false,power,base));assert(MemeAdventureRules::Unlocked(1,true,power,base));
  World w;gSandboxEnabled=false;auto* p=w.add(id);assert(SandboxPlants::IsCustom(p));w.enemy();w.step(100);
  const auto saved=SandboxPlants::SavePower(p);SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,saved));assert(SandboxPlants::SavePower(p)==saved);
  for(int field=0;field<10;++field){auto invalid=saved;invalid[field]=-999;assert(!SandboxPlants::RestorePower(p,invalid));assert(SandboxPlants::SavePower(p)==saved);}
  w.mPaused=true;w.step(1000);assert(SandboxPlants::SavePower(p)==saved);gSandboxEnabled=true;
 }
 for(int id=100;id<120;++id){World w;auto* p=w.plant(1,1);SandboxPlants::Assign(p,id);assert(!SandboxPlants::IsCustom(p)&&!SandboxPlants::Find(id));}
 for(int id=200;id<212;++id)assert(!SandboxZombies::Find(id));
 assert(!Result(120,180)&&!Result(0,183)&&!Result(48,180));
 for(int base=0;base<48;++base)for(int power=180;power<=182;++power)assert(Result(base,power));
 {World w;auto* p=w.add(Result(32,182));w.step(400);assert(SandboxPlants::NativeCanAct(p));SandboxPlants::NativeAction(p);
  assert(!SandboxPlants::NativeCanAct(p));auto* shot=w.AddProjectile(35,-20,0,2,PROJECTILE_PEA);SandboxPlants::OnFired(p,shot,nullptr);
  assert(shot->mPosX==35&&shot->mPosY==-20&&!SandboxPlants::HasShot(shot));assert(SandboxPlants::ShotDamage(shot,40)==120);
  const int saved=SandboxPlants::SaveShot(shot);SandboxPlants::ForgetShot(shot);assert(SandboxPlants::ShotDamage(shot,40)==40);
  SandboxPlants::RestoreShot(shot,saved);assert(SandboxPlants::ShotDamage(shot,40)==120);SandboxPlants::RestoreShot(shot,999);assert(SandboxPlants::ShotDamage(shot,40)==120);}
 {World w;auto* p=w.add(Result(39,181));for(int i=0;i<10;++i)SandboxPlants::NativeAction(p);assert(SandboxPlants::HeatData(p,1)==1000);assert(SandboxPlants::NativeCooldown(p,300)==120);}
 for(int base:{9,38,41}){World w;auto* p=w.add(Result(base,180));assert(SandboxPlants::NativeProduction(p)==1);assert(SandboxPlants::NativeProduction(p)==1);assert(SandboxPlants::NativeProduction(p)==4);}
 for(int base:{2,4,11,12,14,15,17,19,20,27,35}){
  World w;auto* p=w.add(Result(base,181));w.bank.mNumPackets=1;w.bank.mSeedPackets[0].mPacketType=base;w.bank.mSeedPackets[0].mRefreshing=true;
  SandboxPlants::OneShot(p);assert(w.mCoins.mSize==4&&w.bank.mSeedPackets[0].mRefreshCounter==375);const auto saved=SandboxPlants::SavePower(p);
  SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,saved));SandboxPlants::OneShot(p);assert(w.mCoins.mSize==4);
 }
 {World w;auto* p=w.add(Result(20,182));auto* z=w.enemy(120),*far=w.enemy(700);SandboxPlants::OneShot(p);assert(z->chill==600&&far->chill==0);}
 {World w;auto* p=w.add(Result(12,180));auto* z=w.enemy(120),*other=w.enemy(150);SandboxPlants::OneShot(p,z);assert(z->mBodyHealth==1000&&z->mPosX==120&&other->mBodyHealth==920&&other->mPosX==215);}
 for(int base:{16,25,33,37}){World w;auto* p=w.add(Result(base,181));auto* neighbor=w.plant(2,2);neighbor->mPlantHealth=100;w.step(600);assert(neighbor->mPlantHealth>100);}
 {World w;auto* p=w.plant(1,2);p->mSeedType=static_cast<SeedType>(48);p->mImitaterType=static_cast<SeedType>(2);SandboxPlants::Assign(p,Result(2,180));
  assert(SandboxPlants::Power(p)==180);const auto saved=SandboxPlants::SavePower(p);w.step(300);assert(SandboxPlants::SavePower(p)==saved);assert(SandboxPlants::RestorePower(p,saved));}
 for(int base:{0,5,7,18,40}){World w;auto* p=w.add(Result(base,182));w.enemy(500,base==18?1:2);w.step(399);assert(w.mProjectiles.mSize==0);w.step(100);
  assert(w.mProjectiles.mSize==Volley(base)*3*(base==18?3:1));
  for(auto* shot:w.mProjectiles)assert(shot->mProjectileType==(base==5?PROJECTILE_SNOWPEA:PROJECTILE_PEA));
 }
 {World w;auto* p=w.add(Result(0,181));w.enemy();w.step(2000);assert(SandboxPlants::HeatData(p,1)==1000&&w.mProjectiles.mSize>14);}
 {World w;auto* p=w.add(Result(1,182));w.step(2399);assert(w.mCoins.mSize==0);w.step();assert(w.mCoins.mSize==4);}
 {World w;auto* p=w.add(Result(3,182));p->mPlantHealth=2000;w.step(400);assert(p->mPlantHealth==2000);w.step();assert(p->mPlantHealth==2060);p->mPlantHealth-=10;w.step(100);assert(p->mPlantHealth==2050);}
 {World w;auto* p=w.add(Result(23,181));p->mPlantHealth=7000;w.step(600);assert(p->mPlantHealth==7140&&p->mPlantMaxHealth==8000);}
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
