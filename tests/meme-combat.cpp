#include "Engine.h"
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "SandboxArt.h"
#include "SandboxMemeRules.h"
#include "MemeAdventureRules.h"
#include "MemeShooterRules.h"
#include <cmath>
#include <cassert>
#include <iostream>
LawnApp app;LawnApp* gLawnApp=&app;bool gSandboxEnabled=true;
namespace SandboxArt {
Sexy::Image* Image(const char*,const char*){return nullptr;}
Sexy::Image* NativeImage(const char*){return nullptr;}
Sexy::Image* Phone(int){return nullptr;}
Sexy::Image* PhoneHands(const char*){return nullptr;}
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
 // Playable characters have independent, observable mechanics.
 {World w;auto* p=w.add(500);auto* z=w.enemy();p->mPlantHealth=123;w.step(500);
  assert(MemeCharacters::Data(p,1)==80&&w.mProjectiles.mSize==4&&!MemeCharacters::Activate(p));
  w.step(150);assert(MemeCharacters::Data(p,1)==100&&w.mProjectiles.mSize==5);const float zx=z->mPosX;
  z->mDead=true;w.step(200);assert(MemeCharacters::Data(p,1)==100); // no passive rage loss
  assert(MemeCharacters::Activate(p)&&!MemeCharacters::Activate(p));assert(MemeCharacters::Data(p,1)==0&&z->mPosX==zx);
  w.step(57);const auto save=SandboxPlants::SavePower(p);assert(save[8]==35);w.mPaused=true;w.step(300);assert(SandboxPlants::SavePower(p)==save);w.mPaused=false;
  SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,save));w.step(140);
  assert(w.mProjectiles.mSize==55&&MemeCharacters::Data(p,0)==0&&MemeCharacters::Data(p,1)==0&&p->mPlantHealth==123);
  int spread=0;bool up=false,down=false;for(auto* shot:w.mProjectiles)if(MemeCharacters::ShotStyle(shot)==9){++spread;up|=shot->mVelY<0;down|=shot->mVelY>0;assert(SandboxPlants::ShotDamage(shot,20)==20);}
  assert(spread==50&&up&&down);w.step(200);assert(w.mProjectiles.mSize==55);
 }
 {World w;auto* p=w.add(500);w.enemy();w.step(2150);assert(MemeCharacters::Data(p,0)==1&&w.mProjectiles.mSize==15);
  w.step(197);assert(w.mProjectiles.mSize==65&&p->mPlantHealth==300&&MemeCharacters::Data(p,0)==0);
 }
 {World w;auto* p=w.add(500);w.enemy();w.step(650);assert(MemeCharacters::Activate(p));w.mProjectiles.mSize=w.mProjectiles.mMaxSize-8;
  const auto state=SandboxPlants::SavePower(p);w.step(100);assert(SandboxPlants::SavePower(p)[8]==50);
  w.mProjectiles.mSize=5;w.step(197);assert(w.mProjectiles.mSize==55&&p->mPlantHealth==300);
 }
 {World w;auto* p=w.add(500);p->mPlantHealth=99;assert(SandboxPlants::RestorePower(p,{500,99,2,700,350,0,10,0,0,1}));
  assert(MemeCharacters::Data(p,0)==0&&p->mPlantHealth==99&&MemeCharacters::Data(p,1)==0);
  assert(SandboxPlants::RestorePower(p,{500,99,0,400,0,0,10,0,0,1}));assert(MemeCharacters::Data(p,1)==120&&MemeCharacters::Activate(p));
 }
 for(int style=1;style<=9;++style){World w;auto* shot=w.AddProjectile(100,250,0,2,PROJECTILE_PEA);shot->mMotionType=MOTION_STAR;shot->mVelY=0.75f;
  assert(MemeCharacters::RestoreShotStyle(shot,style));SandboxPlants::RestoreShot(shot,(style<<16)|150);
  assert(SandboxPlants::ShotDamage(shot,20)==30&&SandboxPlants::ShotBlastRadius(shot,100)==100);
  float low=250,high=250;for(int i=0;i<140;++i){SandboxPlants::UpdateShot(shot);shot->mPosY+=shot->mVelY;++shot->mProjectileAge;low=std::min(low,shot->mPosY);high=std::max(high,shot->mPosY);}
  if(style<9)assert(low<215&&high>285);else assert(std::abs(shot->mVelY-0.75f)<0.001f);
  const int saved=SandboxPlants::SaveShot(shot);const float y=shot->mPosY,vel=shot->mVelY;const int age=shot->mProjectileAge;
  SandboxPlants::ForgetShot(shot);assert(!MemeCharacters::ShotStyle(shot));SandboxPlants::RestoreShot(shot,saved);
  assert(SandboxPlants::SaveShot(shot)==saved&&shot->mPosY==y&&shot->mVelY==vel&&shot->mProjectileAge==age);
  SandboxPlants::RestoreShot(shot,(10<<16)|100);assert(SandboxPlants::SaveShot(shot)==saved);
  w.mPaused=true;SandboxPlants::UpdateShot(shot);assert(shot->mVelY==vel);w.mPaused=false;shot->mPosY=650;SandboxPlants::UpdateShot(shot);assert(shot->mDead&&!MemeCharacters::ShotStyle(shot));
 }
 assert(MemeShooterRules::OverlapsY(200,24,190,100)&&!MemeShooterRules::OverlapsY(100,24,190,100));
 {World w;auto* p=w.add(501);auto* z=w.enemy(p->mX-30);z->mIsEating=true;w.step(250);assert(z->mBodyHealth==1000);
  p->mRecentlyEatenCountdown=50;w.step();assert(MemeCharacters::Data(p,0)==1);
  const int anchor=p->mX;const float zx=z->mPosX;
  w.step(10);float x=80,y=0,sx=1,sy=1;MemeCharacters::Scale(p,x,y,sx,sy);assert(x<80&&sy<1&&z->mBodyHealth==1000);
  w.step(12);x=80;y=0;sx=sy=1;MemeCharacters::Scale(p,x,y,sx,sy);assert(x>105&&p->mX==anchor&&p->mPlantCol==1);
  assert(z->mBodyHealth==920&&z->mRow==2&&z->mPosX==zx+24);
  const auto saved=SandboxPlants::SavePower(p);w.mPaused=true;w.step(100);assert(SandboxPlants::SavePower(p)==saved);w.mPaused=false;
  SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,saved));w.step(28);x=80;y=0;sx=sy=1;MemeCharacters::Scale(p,x,y,sx,sy);assert(x==80&&y==0&&sx==1&&sy==1&&p->mX==anchor);
  w.step(100);assert(z->mBodyHealth==920);assert(!MemeCharacters::Activate(p));}
 {World w;auto* p=w.add(501);auto* z=w.enemy(p->mX-30,2,ZOMBIE_GARGANTUAR);z->mIsEating=true;p->mRecentlyEatenCountdown=50;
  const float x=z->mPosX;w.step(23);assert(z->mBodyHealth==920&&z->mPosX==x);}
 {World w;auto* p=w.add(501);p->mRecentlyEatenCountdown=50;auto* far=w.enemy(500);far->mIsEating=true;auto* friendZ=w.enemy(60);friendZ->mIsEating=true;friendZ->mMindControlled=true;
  w.step(250);assert(far->mBodyHealth==1000&&friendZ->mBodyHealth==1000&&MemeCharacters::Data(p,0)==0);}
 {World w;auto* p=w.add(504);auto* z=w.enemy(p->mX+140);w.step();assert(MemeCharacters::Hiding(p)&&MemeCharacters::Data(p,0)==1);
  float x=80,y=0,sx=-1,sy=1;w.step(20);MemeCharacters::Scale(p,x,y,sx,sy);assert(x>80&&y<0);
  w.step(20);assert(z->mBodyHealth==920);w.step(40);assert(!MemeCharacters::Hiding(p)&&MemeCharacters::Data(p,0)==0);assert(w.mProjectiles.mSize==0);
  w.step(299);assert(z->mBodyHealth==920);w.step(41);assert(z->mBodyHealth==840);}
 {World w;auto* z=w.enemy();z->mZombieType=static_cast<ZombieType>(5);z->mZombiePhase=PHASE_NEWSPAPER_MAD;z->mPhaseCounter=10;z->mBodyHealth=80;
  SandboxZombies::RecoverPhone(z);assert(z->mShieldHealth==0);z->mPhaseCounter=0;SandboxZombies::RecoverPhone(z);assert(z->mShieldHealth==100&&z->mBodyHealth==80&&z->mZombiePhase==PHASE_NEWSPAPER_READING);
  z->mZombiePhase=PHASE_NEWSPAPER_MAD;z->mShieldHealth=0;z->mHasArm=false;SandboxZombies::RecoverPhone(z);assert(z->mShieldHealth==0);
  z->mHasArm=true;gSandboxEnabled=false;app.adventure=false;SandboxZombies::RecoverPhone(z);assert(z->mShieldHealth==0);gSandboxEnabled=true;app.adventure=true;}
 {World w;auto* p=w.add(502);auto* z=w.enemy(180,1),*heavy=w.enemy(150,3,ZOMBIE_GARGANTUAR);w.step(180);assert(z->mRow==2&&heavy->mRow==3);assert(w.mProjectiles.mSize==0);}
 {World w;auto* p=w.add(503);assert(MemeCharacters::Producing(p));const int clock=p->mLaunchCounter;w.step(20);assert(p->mLaunchCounter==clock);
  auto* z=w.enemy(p->mX+44);w.step();assert(MemeCharacters::Hiding(p)&&!MemeCharacters::Producing(p));
  for(int i=0;i<1110;++i){z->mPosX-=0.1f;w.step();}assert(!MemeCharacters::Hiding(p)&&MemeCharacters::Data(p,0)==2);
  w.step(100);assert(w.mProjectiles.mSize==3&&w.mCoins.mSize==0);
  for(auto* shot:w.mProjectiles)assert(shot->mMotionType==MOTION_BACKWARDS&&SandboxPlants::ShotDamage(shot,20)==60);
  assert(!MemeCharacters::Producing(p));w.step(180);assert(MemeCharacters::Producing(p));}
 {World w;auto* p=w.add(502);w.enemy(p->mX+130);w.step(400);assert(w.mProjectiles.mSize>=3);}
 // A threat can disappear without ever walking behind the flower. Returning
 // to full size/production must not wait out the entire ambush timeout.
 for(int gone=0;gone<5;++gone){World w;auto* p=w.add(503);auto* z=w.enemy(p->mX+20);w.step(30);
  float x=0,y=0,sx=1,sy=1;MemeCharacters::Scale(p,x,y,sx,sy);assert(sy<0.3f&&MemeCharacters::Hiding(p));
  const int hp=p->mPlantHealth,clock=p->mLaunchCounter;
  if(gone==0)z->mDead=true;else if(gone==1)z->mMindControlled=true;else if(gone==2)z->mRow=1;else if(gone==3)z->mHasHead=false;else z->mPosX=p->mX+300;
  w.step(35);assert(MemeCharacters::Producing(p)&&!MemeCharacters::Hiding(p));
  x=y=0;sx=sy=1;MemeCharacters::Scale(p,x,y,sx,sy);assert(x==0&&y==0&&sx==1&&sy==1);
  assert(w.mProjectiles.mSize==0&&p->mPlantHealth==hp&&p->mLaunchCounter==clock);
 }
 // Interrupted collapse, pause/save mid-rise, and old flattened saves all
 // restore the native anchor/scale without healing or resetting production.
 for(int frames:{1,6,16,30}){World w;auto* p=w.add(503);auto* z=w.enemy(p->mX+20);w.step(frames);
  float x=0,y=0,sx=1,sy=1;MemeCharacters::Scale(p,x,y,sx,sy);const float previous=sy;
  z->mDead=true;w.step();x=y=0;sx=sy=1;MemeCharacters::Scale(p,x,y,sx,sy);assert(std::abs(sy-previous)<0.05f);
  const auto state=SandboxPlants::SavePower(p);w.mPaused=true;w.step(100);assert(SandboxPlants::SavePower(p)==state);w.mPaused=false;
  SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,state));
  for(int i=0;i<35;++i){const float last=sy;w.step();x=y=0;sx=sy=1;MemeCharacters::Scale(p,x,y,sx,sy);assert(sy>=last&&sy<=1&&sx>=1&&sx<=1.15f);}
  assert(MemeCharacters::Producing(p)&&sx==1&&sy==1&&x==0&&y==0);
 }
 {World w;auto* p=w.add(503);p->mPlantHealth=123;p->mLaunchCounter=91;
  assert(SandboxPlants::RestorePower(p,{503,123,1,1000,1300,0,70,0,0,1}));w.step(31);
  assert(MemeCharacters::Producing(p)&&p->mPlantHealth==123&&p->mLaunchCounter==91&&w.mProjectiles.mSize==0);
 }
 {World w;auto* p=w.add(503);auto* a=w.enemy(p->mX+20);w.enemy(p->mX+30);w.step(30);a->mDead=true;w.step(100);
  assert(MemeCharacters::Hiding(p));w.step(1301);assert(MemeCharacters::Producing(p)&&w.mProjectiles.mSize==0);
 }
 static_assert(MemeCharacters::ForBase(0)->cost==100&&MemeCharacters::ForBase(1)->cost==50&&MemeCharacters::ForBase(3)->cost==50&&MemeCharacters::ForBase(8)->cost==0);
 static_assert(MemeCharacters::ForBase(0)->unlock==1&&MemeCharacters::ForBase(1)->unlock==2&&MemeCharacters::ForBase(3)->unlock==4&&MemeCharacters::ForBase(8)->unlock==11);
 for(int id:{500,501,502,503,504}){World w;auto* p=w.add(id);w.step(100);auto state=SandboxPlants::SavePower(p);
  w.mPaused=true;w.step(1000);assert(SandboxPlants::SavePower(p)==state);SandboxPlants::Forget(p);assert(!SandboxPlants::IsCustom(p));
  assert(SandboxPlants::RestorePower(p,state)&&SandboxPlants::SavePower(p)==state);state[9]=0;assert(!SandboxPlants::RestorePower(p,state));}
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
