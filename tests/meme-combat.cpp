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
 // Gatling: one 15-damage pea every 10 ticks, 120 shots then exactly 350
 // cooling ticks. Heat/timers are per-plant and pause/save without free shots.
 static_assert(MemeCharacters::GatlingInterval==10&&MemeCharacters::GatlingHeatLimit==120&&MemeCharacters::GatlingCooldown==350);
 {World w;auto* p=w.add(522);w.enemy();p->mPlantHealth=183;
  for(int tick=1;tick<=1200;++tick){w.step();assert(w.mProjectiles.mSize==tick/10);}
  assert(MemeCharacters::Data(p,0)==1&&MemeCharacters::Data(p,1)==120&&MemeCharacters::Data(p,2)==350);
  const auto saved=SandboxPlants::SavePower(p);w.mPaused=true;w.step(400);assert(SandboxPlants::SavePower(p)==saved);w.mPaused=false;
  SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,saved)&&SandboxPlants::SavePower(p)==saved);
  w.step(349);assert(w.mProjectiles.mSize==120&&MemeCharacters::Data(p,2)==1);
  w.step();assert(w.mProjectiles.mSize==121&&MemeCharacters::Data(p,0)==0&&MemeCharacters::Data(p,1)==1&&p->mPlantHealth==183);
  for(auto* shot:w.mProjectiles){
   assert(MemeCharacters::ShotStyle(shot)==MemeCharacters::GatlingProjectile&&MemeCharacters::CanHit(shot));
   assert(shot->mMotionType==MOTION_STRAIGHT&&shot->mVelY==0&&SandboxPlants::ShotDamage(shot,20)==15&&SandboxPlants::ShotScale(shot)==1);
   const int packed=SandboxPlants::SaveShot(shot);SandboxPlants::ForgetShot(shot);SandboxPlants::RestoreShot(shot,packed);
   assert(SandboxPlants::SaveShot(shot)==packed&&SandboxPlants::ShotDamage(shot,20)==15);
   shot->mProjectileType=PROJECTILE_FIREBALL;assert(SandboxPlants::ShotDamage(shot,40)==30);
   SandboxPlants::ForgetShot(shot);SandboxPlants::RestoreShot(shot,packed);assert(SandboxPlants::ShotDamage(shot,40)==30);
  }
  auto* plain=w.AddProjectile(0,200,0,2,PROJECTILE_PEA);assert(SandboxPlants::ShotDamage(plain,20)==20);
 }
 {World w;auto* p=w.add(522);w.step(1500);assert(w.mProjectiles.mSize==0&&MemeCharacters::Data(p,1)==0);
  auto* z=w.enemy();w.step();w.step(360);assert(w.mProjectiles.mSize==37&&MemeCharacters::Data(p,1)==37);
  const auto saved=SandboxPlants::SavePower(p);SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,saved));
  w.step(9);assert(w.mProjectiles.mSize==37);w.step();assert(w.mProjectiles.mSize==38);
  z->mDead=true;w.step(300);assert(w.mProjectiles.mSize==38&&MemeCharacters::Data(p,1)==38);
  z->mDead=false;w.mProjectiles.mMaxSize=w.mProjectiles.mSize+8;w.step(300);assert(MemeCharacters::Data(p,1)==38);
  w.mProjectiles.mMaxSize=10000;p->mIsAsleep=true;w.step(100);assert(w.mProjectiles.mSize==38);p->mIsAsleep=false;
  w.step();assert(w.mProjectiles.mSize==39);auto* other=w.add(522);assert(MemeCharacters::Data(other,1)==0);
  auto bad=SandboxPlants::SavePower(p);bad[2]=1;assert(!SandboxPlants::RestorePower(p,bad));
  bad=SandboxPlants::SavePower(p);bad[4]=350;assert(!SandboxPlants::RestorePower(p,bad));
 }
 // Exactly 50 weak native peas, with independent damage tags and resumable
 // partial volleys. No enemy means no new volley; losing one doesn't cancel it.
 {World w;auto* p=w.add(521);w.step(200);assert(w.mProjectiles.mSize==0);
  auto* z=w.enemy();w.step();assert(w.mProjectiles.mSize==1&&MemeCharacters::Data(p,4)==49);
  w.step(19);assert(w.mProjectiles.mSize==10);const auto saved=SandboxPlants::SavePower(p);
  w.mPaused=true;w.step(100);assert(SandboxPlants::SavePower(p)==saved);w.mPaused=false;
  SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,saved));z->mDead=true;
  w.step(79);assert(w.mProjectiles.mSize==50&&MemeCharacters::Data(p,4)==0&&MemeCharacters::Data(p,0)==0);
  for(auto* shot:w.mProjectiles){
   assert(MemeCharacters::ShotStyle(shot)==MemeCharacters::WeakProjectile&&MemeCharacters::CanHit(shot));
   assert(shot->mMotionType==MOTION_STRAIGHT&&shot->mVelY==0&&SandboxPlants::ShotDamage(shot,20)==1);
   const int packed=SandboxPlants::SaveShot(shot);SandboxPlants::ForgetShot(shot);SandboxPlants::RestoreShot(shot,packed);
   assert(SandboxPlants::SaveShot(shot)==packed&&SandboxPlants::ShotDamage(shot,20)==1);
   shot->mProjectileType=PROJECTILE_FIREBALL;assert(SandboxPlants::ShotDamage(shot,40)==2);
   SandboxPlants::ForgetShot(shot);SandboxPlants::RestoreShot(shot,packed);assert(SandboxPlants::ShotDamage(shot,40)==2);
  }
  auto* native=w.AddProjectile(100,250,0,2,PROJECTILE_PEA);assert(SandboxPlants::ShotDamage(native,20)==20);
  z->mDead=false;w.step(149);assert(w.mProjectiles.mSize==51);w.step();assert(w.mProjectiles.mSize==52);
  const auto mid=SandboxPlants::SavePower(p);w.mProjectiles.mMaxSize=w.mProjectiles.mSize+8;w.step(100);assert(MemeCharacters::Data(p,4)==mid[8]);
  w.mProjectiles.mMaxSize=10000;w.step();assert(MemeCharacters::Data(p,4)==mid[8]-1);
 }
 // Tuck before contact, stay down through the crossing, then fully recover.
 // Production countdown and ordinary sunflowers are untouched by this tick.
 {World w;auto* p=w.add(520);auto* ordinary=w.add(1);const int countdown=p->mLaunchCounter;
  assert(MemeCharacters::Producing(p));w.step(500);assert(p->mLaunchCounter==countdown&&!MemeCharacters::Hiding(p));
  auto* z=w.enemy(400);w.step();assert(!MemeCharacters::Hiding(p));
  z->mPosX=160;w.step();assert(MemeCharacters::Hiding(p)&&!MemeCharacters::Hiding(ordinary));
  z->mPosX=170;w.step();assert(MemeCharacters::Hiding(p)); // release hysteresis
  z->mPosX=200;w.step();assert(!MemeCharacters::Hiding(p));
  z->mPosX=160;w.step();assert(MemeCharacters::Hiding(p));
  const auto saved=MemeCharacters::Save(p);w.mPaused=true;w.step(100);assert(MemeCharacters::Save(p)==saved);w.mPaused=false;
  MemeCharacters::Forget(p);assert(MemeCharacters::Restore(p,saved)&&MemeCharacters::Hiding(p)&&p->mLaunchCounter==countdown);
  for(int x=150;x>=-60;--x){z->mPosX=x;w.step();assert(MemeCharacters::Hiding(p)&&p->mPlantHealth==300);}
  z->mPosX=-100;w.step();assert(!MemeCharacters::Hiding(p));
  for(int i=0;i<8;++i){z->mPosX=80;w.step();assert(MemeCharacters::Hiding(p));z->mDead=true;w.step();assert(!MemeCharacters::Hiding(p));z->mDead=false;}
 }
 // No global/multi-row fear, nor fear of friendly/flying/dead zombies.
 {World w;auto* p=w.add(520);auto* z=w.enemy(80,1);w.step();assert(!MemeCharacters::Hiding(p));
  z->mRow=2;z->mMindControlled=true;w.step();assert(!MemeCharacters::Hiding(p));
  z->mMindControlled=false;z->flying=true;w.step();assert(!MemeCharacters::Hiding(p));
  z->flying=false;z->mBodyHealth=0;w.step();assert(!MemeCharacters::Hiding(p));
  z->mBodyHealth=100;z->mHasHead=false;w.step();assert(MemeCharacters::Hiding(p)); // Louis still scares it.
  p->airborne=true;w.step();assert(!MemeCharacters::Hiding(p));p->airborne=false;
  p->mIsAsleep=true;w.step();assert(!MemeCharacters::Hiding(p));p->mIsAsleep=false;w.step();assert(MemeCharacters::Hiding(p));
  p->mSquished=true;w.step();assert(!MemeCharacters::Hiding(p));
 }
 // Tucking skips chewing, not the need for a lily pad. Losing the final
 // support removes both standing and hidden flowers, including loaded saves.
 for(bool hidden:{false,true}){World w;w.pool=true;auto* p=w.add(520);auto* pad=w.add(16);
  if(hidden)w.enemy(80);w.step();assert(!p->mDead&&MemeCharacters::Hiding(p)==hidden);
  const auto saved=MemeCharacters::Save(p);const int countdown=p->mLaunchCounter;
  pad->Die();w.mPaused=true;w.step();assert(!p->mDead);w.mPaused=false;
  SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,saved));
  w.step();assert(p->mDead&&p->mLaunchCounter==countdown&&!MemeCharacters::Is(p));
 }
 // Other rows/cells and dead, crushed or carried pads cannot support it.
 for(int invalid=0;invalid<7;++invalid){World w;w.pool=true;auto* p=w.add(520);auto* pad=w.add(16);
  if(invalid==0)pad->mRow=3;if(invalid==1)pad->mPlantCol=2;if(invalid==2)pad->mDead=true;
  if(invalid==3)pad->mSquished=true;if(invalid==4)pad->airborne=true;if(invalid==5)pad->mPlantHealth=0;
  if(invalid==6)pad->mSeedType=static_cast<SeedType>(33); // A flower pot is not a lily pad.
  w.step();assert(p->mDead);
 }
 // Sandbox stacks survive while any valid pad remains; an imitater pad is
 // treated as support during its native transformation too.
 {World w;w.pool=true;auto* p=w.add(520);auto* first=w.add(16);auto* second=w.add(16);
  first->Die();w.step();assert(!p->mDead);second->mSeedType=SEED_IMITATER;second->mImitaterType=SEED_LILYPAD;
  w.step();assert(!p->mDead);second->Die();w.step();assert(p->mDead);
 }
 // Scope: land plants and lifted/display plants are unaffected. A flower
 // returned to unsupported water is cleaned up, even if sleeping.
 {World w;w.pool=true;auto* land=w.add(520,1);auto* lifted=w.add(520);lifted->airborne=true;
  auto* preview=w.add(520);preview->inPlay=false;auto* other=w.add(500);
  w.step();assert(!land->mDead&&!lifted->mDead&&!preview->mDead&&!other->mDead);
  lifted->airborne=false;lifted->mIsAsleep=true;w.step();assert(lifted->mDead&&!land->mDead&&!preview->mDead&&!other->mDead);
 }
 // Migrate legacy embarrassment/pending-sun saves without reviving the old
 // mechanic or changing health/production. New hidden saves round-trip.
 {World w;auto* p=w.add(520);p->mPlantHealth=177;p->mLaunchCounter=821;
  p->mSeedType=static_cast<SeedType>(53); // Previous build's separate card.
  assert(MemeCharacters::Restore(p,{520,177,1,22,300,300,100,1,2,1}));
  assert(p->mSeedType==SEED_SUNFLOWER);
  assert((MemeCharacters::Save(p)==std::array<int,10>{520,177,0,0,0,0,100,0,0,3}));
  assert(p->mPlantHealth==177&&p->mLaunchCounter==821&&!MemeCharacters::Hiding(p));
  auto corrupt=MemeCharacters::Save(p);corrupt[4]=1;assert(!MemeCharacters::Restore(p,corrupt));
  corrupt=MemeCharacters::Save(p);corrupt[2]=2;assert(!MemeCharacters::Restore(p,corrupt));
 }
 using namespace SandboxMemeRules;
 static_assert(MemeShooterRules::BurstCount==50&&MemeShooterRules::MaxRage==300&&MemeShooterRules::PerShot==20&&MemeShooterRules::BurstTicks==300);
 static_assert(MemeShooterRules::RecoveryDelay==300);
 for(int roll=0;roll<=60;++roll)assert(std::abs(MemeShooterRules::BurstSpeed(roll)-(4.6f+roll*0.05f)*0.75f)<0.0001f);
 {World w;auto* p=w.add(500);w.enemy();w.step(2450);const int shots=w.mProjectiles.mSize;assert(shots==65);w.step(299);assert(w.mProjectiles.mSize==shots);w.step();assert(w.mProjectiles.mSize==shots+1&&MemeCharacters::Data(p,1)==20);}
 {int ticks=0;for(int fired=0;fired<50;++fired){const int delay=MemeShooterRules::BurstInterval(fired);assert(delay==6);ticks+=delay;assert(ticks==(fired+1)*6);}assert(ticks==300);}
 {World w;auto* p=w.add(500);assert(SandboxPlants::RestorePower(p,{500,300,0,100,0,50,40,0,0,2}));
  const auto state=SandboxPlants::SavePower(p);w.mPaused=true;w.step(100);
  assert(SandboxPlants::SavePower(p)==state);
 }
 // Neither the body nor the bar can release rage, including pot/pool offsets.
 for(int offset:{-5,-2,0,2}){World w;auto* p=w.add(500);p->drawHeightOffset=offset;
  assert(SandboxPlants::RestorePower(p,{500,300,0,100,0,50,40,0,0,2}));
  const auto state=SandboxPlants::SavePower(p);
  const int x=p->mX+40,y=p->mY+offset+82;
  w.mPaused=true;assert(!MemeCharacters::Click(&w,x,y));w.mPaused=false;
  p->mIsAsleep=true;assert(!MemeCharacters::Click(&w,x,y));p->mIsAsleep=false;
  p->airborne=true;assert(!MemeCharacters::Click(&w,x,y));p->airborne=false;
  assert(!MemeCharacters::Click(&w,p->mX-1,y)&&!MemeCharacters::Click(&w,p->mX+80,y));
  assert(!MemeCharacters::Click(&w,x,p->mY+offset-1)&&!MemeCharacters::Click(&w,x,p->mY+offset+86));
  assert(!MemeCharacters::Click(&w,x,y)&&!MemeCharacters::Click(&w,x,p->mY+offset+40));
  assert(!MemeCharacters::Activate(p)&&SandboxPlants::SavePower(p)==state&&p->mPlantHealth==300);
 }
 {World w;auto* unready=w.add(500);auto* ready=w.add(500);
  assert(SandboxPlants::RestorePower(ready,{500,300,0,100,0,50,40,0,0,2}));
  assert(!MemeCharacters::Click(&w,ready->mX+40,ready->mY+82));
  assert(MemeCharacters::Data(unready,0)==0&&MemeCharacters::Data(ready,0)==0&&MemeCharacters::Data(ready,1)==100);
 }
 // Playable characters have independent, observable mechanics.
 {World w;auto* p=w.add(500);auto* z=w.enemy();const int sounds=app.rageReleaseRequests;p->mPlantHealth=123;w.step(500);
  assert(MemeCharacters::Data(p,1)==80&&w.mProjectiles.mSize==4&&!MemeCharacters::Activate(p));
  assert(app.rageReleaseRequests==sounds);
  w.step(150);assert(MemeCharacters::Data(p,1)==100&&w.mProjectiles.mSize==5);const float zx=z->mPosX;
  z->mDead=true;w.step(200);assert(MemeCharacters::Data(p,1)==100); // no passive rage loss
  assert(!MemeCharacters::Activate(p)&&!MemeCharacters::Click(&w,p->mX+40,p->mY+40));assert(MemeCharacters::Data(p,1)==100&&z->mPosX==zx);
  z->mDead=false;w.step(1351);assert(MemeCharacters::Data(p,0)==1&&MemeCharacters::Data(p,4)==50&&w.mProjectiles.mSize==15);z->mDead=true;
  assert(app.rageReleaseRequests==sounds+1);
  w.step(57);const auto save=SandboxPlants::SavePower(p);assert(save[8]==41);w.mPaused=true;w.step(300);assert(SandboxPlants::SavePower(p)==save);w.mPaused=false;
  SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,save));w.step(242);assert(MemeCharacters::Data(p,4)==1);w.step();
  assert(w.mProjectiles.mSize==65&&MemeCharacters::Data(p,0)==0&&MemeCharacters::Data(p,1)==0&&p->mPlantHealth==123);
  int spread=0;bool up=false,down=false;for(auto* shot:w.mProjectiles)if(MemeCharacters::ShotStyle(shot)==9){++spread;up|=shot->mVelY<0;down|=shot->mVelY>0;assert(SandboxPlants::ShotDamage(shot,20)==20);}
  assert(spread==50&&up&&down);w.step(200);assert(w.mProjectiles.mSize==65);
  assert(app.rageReleaseRequests==sounds+1); // no sound per pea/load
 }
 {World w;auto* p=w.add(500);const int sounds=app.rageReleaseRequests;w.enemy();
  w.step(1400);assert(MemeCharacters::Data(p,1)==200&&MemeCharacters::Data(p,0)==0&&app.rageReleaseRequests==sounds);
  w.step(749);assert(MemeCharacters::Data(p,1)==280&&MemeCharacters::Data(p,0)==0);w.step();assert(MemeCharacters::Data(p,0)==1&&w.mProjectiles.mSize==15);
  assert(app.rageReleaseRequests==sounds+1);
  // All 50 native projectiles are paced across exactly three simulation seconds.
  for(int tick=1;tick<=300;++tick){w.step();assert(w.mProjectiles.mSize==15+tick/6);assert(MemeCharacters::Data(p,4)==50-tick/6);assert(MemeCharacters::Data(p,0)==(tick<300?1:0));}
  assert(w.mProjectiles.mSize==65&&p->mPlantHealth==300);
  assert(app.rageReleaseRequests==sounds+1);
 }
 {World w;auto* p=w.add(500);w.enemy();w.step(2150);assert(MemeCharacters::Data(p,0)==1);w.mProjectiles.mSize=w.mProjectiles.mMaxSize-8;
  w.step(100);assert(SandboxPlants::SavePower(p)[8]==50);
  w.mProjectiles.mSize=15;w.step(300);assert(w.mProjectiles.mSize==65&&p->mPlantHealth==300);
 }
 {World w;auto* p=w.add(500);p->mPlantHealth=99;assert(SandboxPlants::RestorePower(p,{500,99,2,700,350,0,10,0,0,1}));
  assert(MemeCharacters::Data(p,0)==0&&p->mPlantHealth==99&&MemeCharacters::Data(p,1)==0);
  assert(SandboxPlants::RestorePower(p,{500,99,0,400,0,0,10,0,0,1}));assert(MemeCharacters::Data(p,1)==120&&!MemeCharacters::Activate(p));
 }
 {World w;auto* p=w.add(500);const int sounds=app.rageReleaseRequests;
  // Preserve saved partial rage verbatim: 280 still needs one normal shot.
  assert(SandboxPlants::RestorePower(p,{500,300,0,280,0,100,10,0,0,2}));
  assert(MemeCharacters::Data(p,1)==280&&app.rageReleaseRequests==sounds);
  w.mPaused=true;w.step(100);assert(MemeCharacters::Data(p,0)==0);w.mPaused=false;
  w.step(100);assert(MemeCharacters::Data(p,1)==280&&w.mProjectiles.mSize==0);
  w.enemy();w.step();assert(MemeCharacters::Data(p,0)==1&&app.rageReleaseRequests==sounds+1);
  w.step(300);assert(w.mProjectiles.mSize==51&&MemeCharacters::Data(p,0)==0);
 }
 {World w;auto* p=w.add(500);const int sounds=app.rageReleaseRequests;
  assert(SandboxPlants::RestorePower(p,{500,300,0,300,0,100,10,0,0,2}));
  w.mPaused=true;w.step(100);assert(MemeCharacters::Data(p,0)==0);w.mPaused=false;
  w.step(300);assert(w.mProjectiles.mSize==49&&MemeCharacters::Data(p,0)==1);w.step();
  assert(w.mProjectiles.mSize==50&&MemeCharacters::Data(p,0)==0&&app.rageReleaseRequests==sounds+1);
 }
 {World w;auto* p=w.add(500);const int sounds=app.rageReleaseRequests;
  // An unfinished volley resumes its remainder, never tops up to 50.
  assert(SandboxPlants::RestorePower(p,{500,300,1,0,0,0,10,0,39,2}));
  w.step(300);assert(w.mProjectiles.mSize==39&&MemeCharacters::Data(p,0)==0&&app.rageReleaseRequests==sounds);
 }
 for(int oldRemaining:{75,80,107}){World w;auto* p=w.add(500);const int sounds=app.rageReleaseRequests;p->mPlantHealth=123;
  // Old 80/150-pea saves still load; their remainder is capped, never refilled.
  assert(SandboxPlants::RestorePower(p,{500,123,1,0,0,0,10,0,oldRemaining,2}));assert(MemeCharacters::Data(p,4)==50);
  w.step(300);assert(w.mProjectiles.mSize==50&&MemeCharacters::Data(p,0)==0&&p->mPlantHealth==123&&app.rageReleaseRequests==sounds);
 }
 for(int style=1;style<=9;++style){World w;auto* shot=w.AddProjectile(100,250,0,2,PROJECTILE_PEA);shot->mMotionType=MOTION_STAR;shot->mVelY=0.75f;
  assert(MemeCharacters::RestoreShotStyle(shot,style));SandboxPlants::RestoreShot(shot,(style<<16)|150);
  assert(SandboxPlants::ShotDamage(shot,20)==30&&SandboxPlants::ShotBlastRadius(shot,100)==100);
  float low=250,high=250;for(int i=0;i<140;++i){SandboxPlants::UpdateShot(shot);shot->mPosY+=shot->mVelY;++shot->mProjectileAge;low=std::min(low,shot->mPosY);high=std::max(high,shot->mPosY);}
  if(style<9)assert(low<215&&high>285);else assert(std::abs(shot->mVelY-0.75f)<0.001f);
  const int saved=SandboxPlants::SaveShot(shot);const float y=shot->mPosY,vel=shot->mVelY;const int age=shot->mProjectileAge;
  SandboxPlants::ForgetShot(shot);assert(!MemeCharacters::ShotStyle(shot));SandboxPlants::RestoreShot(shot,saved);
  assert(SandboxPlants::SaveShot(shot)==saved&&shot->mPosY==y&&shot->mVelY==vel&&shot->mProjectileAge==age);
  SandboxPlants::RestoreShot(shot,(21<<16)|100);assert(SandboxPlants::SaveShot(shot)==saved);
  w.mPaused=true;SandboxPlants::UpdateShot(shot);assert(shot->mVelY==vel);w.mPaused=false;shot->mPosY=650;SandboxPlants::UpdateShot(shot);assert(shot->mDead&&!MemeCharacters::ShotStyle(shot));
 }
 assert(MemeShooterRules::OverlapsY(200,24,190,100)&&!MemeShooterRules::OverlapsY(100,24,190,100));
 for(int style=0;style<=20;++style)assert(MemeShooterRules::UsesFreeAim(style)==(style>=1&&style<=9));
 {World w;auto* p=w.add(501);auto* z=w.enemy(p->mX-30);z->mIsEating=true;w.step(250);assert(z->mBodyHealth==1000);
  p->mRecentlyEatenCountdown=50;w.step();assert(MemeCharacters::Data(p,0)==1);
  const int anchor=p->mX;const float zx=z->mPosX;
  w.step(10);float x=80,y=0,sx=1,sy=1;MemeCharacters::Scale(p,x,y,sx,sy);assert(x<80&&sy<1&&z->mBodyHealth==1000);
  w.step(12);x=80;y=0;sx=sy=1;MemeCharacters::Scale(p,x,y,sx,sy);assert(x>105&&p->mX==anchor&&p->mPlantCol==1);
  assert(z->mBodyHealth==920&&z->mRow==2&&z->mPosX==zx&&z->mIsEating);
  const auto saved=SandboxPlants::SavePower(p);w.mPaused=true;w.step(100);assert(SandboxPlants::SavePower(p)==saved);w.mPaused=false;
  SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,saved));w.step(28);x=80;y=0;sx=sy=1;MemeCharacters::Scale(p,x,y,sx,sy);assert(x==80&&y==0&&sx==1&&sy==1&&p->mX==anchor);
  w.step(100);assert(z->mBodyHealth==920);assert(!MemeCharacters::Activate(p));}
 {World w;auto* p=w.add(501);auto* z=w.enemy(p->mX-30,2,ZOMBIE_GARGANTUAR);z->mIsEating=true;p->mRecentlyEatenCountdown=50;
  const float x=z->mPosX;w.step(23);assert(z->mBodyHealth==920&&z->mPosX==x);}
 {World w;auto* p=w.add(501);auto* z=w.enemy(p->mX-30);z->mIsEating=true;p->mRecentlyEatenCountdown=50;
  const float x=z->mPosX;w.step(23);assert(z->mBodyHealth==920);w.step(299);assert(z->mBodyHealth==920);w.step();assert(z->mBodyHealth==840&&z->mPosX==x&&z->mIsEating);
 }
 // Accuracy is decided once per ordinary projectile, independent of enemies
 // or range; a miss remains a miss after conversion and save restoration.
 {World w;auto* p=w.add(500);int hits=0;
  for(int roll=0;roll<100;++roll){Sexy::forcedRoll=roll%10;auto* s=w.AddProjectile(100,250,0,2,PROJECTILE_PEA);SandboxPlants::OnFired(p,s,nullptr);
   const bool hit=MemeCharacters::CanHit(s);hits+=hit;const int saved=SandboxPlants::SaveShot(s);s->mProjectileType=PROJECTILE_FIREBALL;
   SandboxPlants::ForgetShot(s);SandboxPlants::RestoreShot(s,saved);assert(MemeCharacters::CanHit(s)==hit);
  }Sexy::forcedRoll=-1;assert(hits==10);
 }
 {World w;auto* p=w.add(500);int up=0,down=0;float low=100,high=0;
  for(int i=0;i<100;++i){auto* s=w.AddProjectile(100,250,0,2,PROJECTILE_PEA);SandboxPlants::OnFired(p,s,nullptr);
   if(MemeCharacters::ShotStyle(s)==10)continue;
   const int style=MemeCharacters::ShotStyle(s);assert(MemeShooterRules::IsFloating(style)&&!MemeCharacters::CanHit(s));const float vx=s->mVelX;
   up+=s->mVelY<0;down+=s->mVelY>0;
   for(int age=0;age<130;++age){s->mProjectileAge=age;SandboxPlants::UpdateShot(s);assert(s->mVelX==vx);s->mPosY+=s->mVelY;assert(std::abs(s->mPosY-250-MemeShooterRules::FloatingOffset(style,age+1))<0.001f);}
   for(int turn=1;turn<6;++turn){const float a=std::abs(MemeShooterRules::FloatingPeak(style,turn));low=std::min(low,a);high=std::max(high,a);assert(MemeShooterRules::FloatingPeak(style,turn)*MemeShooterRules::FloatingPeak(style,turn+1)<0);}
   const float y=s->mPosY,vy=s->mVelY;
   const int saved=SandboxPlants::SaveShot(s);SandboxPlants::ForgetShot(s);SandboxPlants::RestoreShot(s,saved);
   w.mPaused=true;SandboxPlants::UpdateShot(s);w.mPaused=false;assert(s->mVelX==vx&&s->mVelY==vy&&!MemeCharacters::CanHit(s));
   assert(s->mPosY==y&&MemeCharacters::ShotStyle(s)==style);s->mProjectileAge=130;SandboxPlants::UpdateShot(s);assert(s->mVelY==MemeShooterRules::FloatingStep(style,130));
  }assert(up>10&&down>10&&high-low>40);
 }
 // Every saved floating seed alternates extrema, stays bounded, and resumes
 // the next step exactly. Old style-20 diagonal peas remain unchanged.
 for(int style=MemeShooterRules::FloatingFirst;style<=MemeShooterRules::FloatingLast;++style){World w;auto* s=w.AddProjectile(100,250,0,2,PROJECTILE_PEA);s->mMotionType=MOTION_STAR;
  assert(MemeCharacters::RestoreShotStyle(s,style));float offset=0;
  for(int age=0;age<360;++age){s->mProjectileAge=age;SandboxPlants::UpdateShot(s);offset+=s->mVelY;assert(std::abs(offset)<=76.001f&&std::abs(offset-MemeShooterRules::FloatingOffset(style,age+1))<0.001f);}
  s->mProjectileType=PROJECTILE_FIREBALL;const int saved=SandboxPlants::SaveShot(s);SandboxPlants::ForgetShot(s);SandboxPlants::RestoreShot(s,saved);assert(MemeCharacters::ShotStyle(s)==style&&!MemeCharacters::CanHit(s));
  for(int invalid:{21,31,288,32767}){assert(!MemeCharacters::RestoreShotStyle(s,invalid));assert(MemeCharacters::ShotStyle(s)==style);}
 }
 {World w;auto* s=w.AddProjectile(100,250,0,2,PROJECTILE_PEA);s->mMotionType=MOTION_STAR;s->mVelY=-0.7f;
  assert(MemeCharacters::RestoreShotStyle(s,20));for(int age=0;age<100;++age){s->mProjectileAge=age;SandboxPlants::UpdateShot(s);assert(s->mVelY==-0.7f);}
 }
 {World w;auto* p=w.add(500);w.enemy();w.step(2150);w.step(300);
  float minSpeed=100,maxSpeed=0;int turns=0;float last=0;
  for(auto* s:w.mProjectiles)if(MemeCharacters::ShotStyle(s)==9){float speed=std::hypot(s->mVelX,s->mVelY);minSpeed=std::min(minSpeed,speed);maxSpeed=std::max(maxSpeed,speed);turns+=(last*s->mVelY<0);last=s->mVelY;}
  assert(maxSpeed-minSpeed>2.0f&&turns>15);
 }

 // Removed originals cannot be assigned, restored or re-entered through legacy powers.
 static_assert(MemeCharacters::Definitions.size()==6&&SandboxPlants::Definitions.size()==6);
 {World w;auto* p=w.add(519);const int x=p->mX,y=p->mY;w.step(100);assert(w.mProjectiles.mSize==0);
  auto* z=w.enemy();w.step();assert(w.mProjectiles.mSize==1);w.step(149);assert(w.mProjectiles.mSize==1);w.step();assert(w.mProjectiles.mSize==2);
  auto* shot=w.mProjectiles.values[0];assert(shot->mVelX>0&&shot->mVelY==0&&shot->mMotionType==MOTION_STAR);
  assert(MemeCharacters::ShotStyle(shot)==296&&MemeCharacters::CanHit(shot)&&SandboxPlants::ShotDamage(shot,20)==20);
  assert(p->mX==x&&p->mY==y&&p->mPlantHealth==300&&w.mPlants.mSize==1&&!MemeCharacters::Hiding(p));
  const auto saved=SandboxPlants::SavePower(p);const int packed=SandboxPlants::SaveShot(shot);
  SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,saved));SandboxPlants::ForgetShot(shot);SandboxPlants::RestoreShot(shot,packed);assert(MemeCharacters::ShotStyle(shot)==296);
  z->mDead=true;w.step(300);assert(w.mProjectiles.mSize==2);w.mPaused=true;const auto paused=SandboxPlants::SavePower(p);w.step(500);assert(SandboxPlants::SavePower(p)==paused);
 }
 for(int id=502;id<=518;++id){World w;auto* p=w.plant(1,2);p->mSeedType=static_cast<SeedType>(MemeCharacters::RetiredBase(id));
  SandboxPlants::Assign(p,id);assert(!SandboxPlants::IsCustom(p)&&!SandboxPlants::Find(id)&&!MemeCharacters::Find(id));
  assert(!SandboxPlants::RestorePower(p,{id,300,0,0,0,50,0,0,0,1}));
 }
 for(int power=180;power<=182;++power)for(int base:Bases){World w;auto* p=w.plant(1,2);p->mSeedType=static_cast<SeedType>(base);
  const int id=Result(base,power);SandboxPlants::Assign(p,id);assert(!SandboxPlants::IsCustom(p)&&!SandboxPlants::Find(id));
  assert(!SandboxPlants::RestorePower(p,{id,300,0,0,0,50,0,0,0,1}));
 }
 for(int id=502;id<=518;++id)for(bool night:{false,true}){World w;w.night=night;auto* p=w.plant(1,2);
  p->mSeedType=static_cast<SeedType>(id==504?52:MemeCharacters::RetiredBase(id));p->mPlantHealth=123;p->mLaunchCounter=9999;
  if(id==504||id==518){p->mX+=90;p->mY-=50;}
  SandboxPlants::RestoreRetired(p,{id,123,1,0,50,999,20,5,1,1});
  assert(p->mPlantHealth==123&&!SandboxPlants::IsCustom(p)&&p->mLaunchCounter==150&&p->mShootingCounter==0);
  assert(int(p->mSeedType)==MemeCharacters::RetiredBase(id));
  if(id==504||id==518)assert(p->mX==80&&p->mY==200);
  if(id==502)assert(p->mIsAsleep==!night);
 }
 for(int id:{500,501,519,520,521,522}){World w;auto* p=w.add(id);w.step(100);const auto saved=SandboxPlants::SavePower(p);
  for(int field=0;field<10;++field){auto bad=saved;bad[field]=-999;assert(!SandboxPlants::RestorePower(p,bad));assert(SandboxPlants::SavePower(p)==saved);}
  SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,saved)&&SandboxPlants::SavePower(p)==saved);
 }
 // Every formerly modified native zombie retains its armor, position and phase.
 static_assert(SandboxZombies::Definitions.size()==3&&SandboxZombies::Find(212)->base==0&&SandboxZombies::Find(213)->base==0&&SandboxZombies::Find(214)->base==2);
 for(int id=200;id<212;++id)assert(!SandboxZombies::Find(id));
 for(int level:{1,2,3,8,20,50})for(int base:{0,1,2,4,23})for(int wave=-3;wave<30;++wave){
  assert(SandboxZombies::LouisWave(level,base,wave)==(level>=3&&base==0&&wave>=0&&wave%3==0));
  assert(SandboxZombies::RunnerWave(level,base,wave)==(level>=6&&base==0&&wave>=0&&wave%4==1));
  assert(SandboxZombies::ConeWrapWave(level,base,wave)==(level>=16&&base==2&&wave>=3&&wave%4==3));
 }
 // Seven native cone armor units, not seven bodies or damage reduction.
 // Injury, broken armor and identity round-trip without healing on Restore.
 {World w;auto* z=w.enemy(700,2,static_cast<ZombieType>(2));SandboxZombies::Assign(z,214);
  assert(SandboxZombies::IsConeWrap(z)&&z->mBodyHealth==270&&z->mBodyMaxHealth==270);
  assert(z->mHelmHealth==2590&&z->mHelmMaxHealth==2590&&SandboxZombies::Speed(z)==.6f);
  for(int pieces=7;pieces>0;--pieces){
   for(int part=0;part<7;++part)assert(SandboxZombies::ConePartHealth(z->mHelmHealth,part)==(part<pieces?370:0));
   z->TakeDamage(370,0);assert(z->mBodyHealth==270&&z->mHelmHealth==(pieces-1)*370);
   SandboxZombies::Forget(z);assert(SandboxZombies::Restore(z,214)&&z->mHelmHealth==(pieces-1)*370);
  }
  z->TakeDamage(20,0);assert(z->mBodyHealth==250);SandboxZombies::Forget(z);
  assert(SandboxZombies::Restore(z,214)&&z->mBodyHealth==250&&z->mHelmHealth==0&&SandboxZombies::Speed(z)==.6f);
  auto* ordinary=w.enemy(700,2,static_cast<ZombieType>(2));assert(SandboxZombies::Speed(ordinary)==1&&!SandboxZombies::IsConeWrap(ordinary));
  assert(!SandboxZombies::Restore(w.enemy(),214));z->mDead=true;assert(!SandboxZombies::Restore(z,214));
 }
 for(int health=1;health<=2590;++health){int total=0;for(int part=0;part<7;++part){const int hp=SandboxZombies::ConePartHealth(health,part);total+=hp;assert(hp>=0&&hp<=370);}assert(total==health);}
 assert(SandboxZombies::ConeDamageStage(370)==0&&SandboxZombies::ConeDamageStage(200)==1&&SandboxZombies::ConeDamageStage(90)==2);
 for(int part=0;part<20;++part){assert(SandboxZombies::ConeVisualHealth(2590,part)==370);assert(SandboxZombies::ConeVisualHealth(0,part)==0);}
 {World w;auto* z=w.enemy();z->mBodyHealth=z->mBodyMaxHealth=270;
  Reanimation rig;Track tracks[]={{"anim_head1"},{"anim_head2"},{"anim_hair"},{"anim_tongue"},{"anim_body"},{"anim_hand"},{"anim_foot"}};TrackInstance instances[7];
  rig.def.mTracks={7,tracks};rig.mTrackInstances=instances;gLawnApp->reanims[91]=&rig;z->mBodyReanimID=91;
  assert(SandboxZombies::Restore(z,212));assert(SandboxZombies::Type(z)==212&&SandboxZombies::IsLouis(z)&&z->headHides==1);
  for(int i=0;i<7;++i)assert(instances[i].mRenderGroup==(i<4?RENDER_GROUP_HIDDEN:RENDER_GROUP_NORMAL));
  w.step(3000);assert(z->mBodyHealth==270&&z->mHasHead&&SandboxZombies::Speed(z)==1&&SandboxZombies::Damage(z,20,0)==20);
  z->mIsEating=true;SandboxZombies::RefreshDamageArt(z);assert(z->mIsEating&&z->headHides==2&&z->mHasHead);
  z->mBodyHealth=42;z->mHasHead=false;SandboxZombies::Forget(z);assert(SandboxZombies::Type(z)==0);
  assert(SandboxZombies::Restore(z,212)&&z->mBodyHealth==42&&!z->mHasHead); // Never heal or resurrect while loading.
  SandboxZombies::Reset();assert(SandboxZombies::Type(z)==0&&!SandboxZombies::IsLouis(z));
  for(int id=200;id<212;++id)assert(!SandboxZombies::Restore(z,id));
  z->mZombieType=static_cast<ZombieType>(4);assert(!SandboxZombies::Restore(z,212));
  z->mZombieType=ZOMBIE_NORMAL;z->mDead=true;assert(!SandboxZombies::Restore(z,212));
  gLawnApp->reanims.erase(91);
 }
 // The fake breach is positional, not a damage/speed reskin. Native serialized
 // phase/target/facing survive Restore without a second inward charge.
 for(int lastCol:{0,1,3,6}){World w;auto* p=w.plant(lastCol,2);w.plant(8,2);w.plant(0,1);
  auto* z=w.enemy(780);z->mBodyHealth=z->mBodyMaxHealth=270;SandboxZombies::Assign(z,213);
  assert(z->mHasHead&&z->headHides==0&&SandboxZombies::IsRunning(z)&&z->mTargetCol==-1);
  int ticks=0;while(z->mZombiePhase==SandboxZombies::RunIn){const float before=z->mPosX;assert(SandboxZombies::UpdateRunner(z));assert(z->mPosX<=before&&z->mPosX>=40&&++ticks<400);}
  assert(z->mTargetCol==lastCol&&z->mPosX==std::max(40,lastCol*80-25)&&z->mPhaseCounter==24);
  const float turn=z->mPosX;z->mIceTrapCounter=100;for(int i=0;i<70;++i)assert(SandboxZombies::UpdateRunner(z));
  assert(z->mPhaseCounter==24&&z->mPosX==turn);z->mIceTrapCounter=0;
  for(int i=0;i<24;++i){SandboxZombies::UpdateRunner(z);assert(z->mPosX==turn);}
  assert(z->mZombiePhase==SandboxZombies::RunOut&&SandboxZombies::IsRetreating(z));
  z->mBodyHealth=123;SandboxZombies::Forget(z);assert(SandboxZombies::Restore(z,213));
  assert(z->mBodyHealth==123&&z->mZombiePhase==SandboxZombies::RunOut&&z->mTargetCol==lastCol&&z->mHasObject);
  SandboxZombies::RestoreNative(&w);assert(z->mZombiePhase==SandboxZombies::RunOut);
  ticks=0;while(z->mPosX<=850){const float before=z->mPosX;assert(SandboxZombies::UpdateRunner(z));assert(z->mPosX>before&&++ticks<300);}
  assert(p->mPlantHealth==300&&z->mBodyHealth==123&&!z->mIsEating);
 }
 {World w;auto* z=w.enemy(700);SandboxZombies::Assign(z,213);z->chill=100;
  SandboxZombies::UpdateRunner(z);assert(std::abs(z->mPosX-698.6f)<0.01f);
  z->mButteredCounter=100;for(int i=0;i<100;++i)SandboxZombies::UpdateRunner(z);assert(std::abs(z->mPosX-698.6f)<0.01f);
  z->mButteredCounter=0;z->chill=0;SandboxZombies::UpdateRunner(z);assert(std::abs(z->mPosX-695.8f)<0.01f);
  z->mHasHead=false;assert(!SandboxZombies::UpdateRunner(z)&&z->mZombiePhase==PHASE_ZOMBIE_NORMAL);
  z->mDead=true;assert(!SandboxZombies::UpdateRunner(z));
 }
 {World w;auto* z=w.enemy(650);SandboxZombies::Assign(z,213);z->movementBlocked=true;
  for(int i=0;i<300;++i)assert(SandboxZombies::UpdateRunner(z));assert(z->mPosX==650&&z->mTargetCol==-1);
  z->movementBlocked=false;SandboxZombies::UpdateRunner(z);assert(z->mPosX<650);
 }
 {World w;w.plant(8,2);auto* z=w.enemy(50);SandboxZombies::Assign(z,213);SandboxZombies::UpdateRunner(z);assert(z->mPosX==50&&z->mZombiePhase==SandboxZombies::RunBrake);}
 {World w;auto* z=w.enemy(700);SandboxZombies::Assign(z,213);Reanimation body;SandboxZombies::AdjustPose(z,&body);assert(body.mOverlayMatrix.m01>0&&body.mOverlayMatrix.m02<0);
  z->mZombiePhase=SandboxZombies::RunOut;z->mHasObject=true;body.mOverlayMatrix={};SandboxZombies::AdjustPose(z,&body);assert(body.mOverlayMatrix.m01<0&&body.mOverlayMatrix.m02>0);
 }
 for(int type:{0,1,2,3,4,5,6,7,16,21,24}){World w;auto* z=w.enemy(500,2,static_cast<ZombieType>(type));auto* buddy=w.enemy(450);
  z->mBodyHealth=321;z->mHelmHealth=75;z->mShieldHealth=80;z->mIsEating=true;
  for(int age=0;age<1800;++age){z->mZombieAge=age;w.step();assert(SandboxZombies::Speed(z)==1);
   assert(!SandboxZombies::IsResting(z)&&!SandboxZombies::IsRetreating(z)&&!SandboxZombies::IsFeigning(z)&&!SandboxZombies::IsPhone(z));}
  assert(z->mBodyHealth==321&&z->mHelmHealth==75&&z->mShieldHealth==80&&z->mPosX==500&&z->mRow==2&&z->mIsEating);
  assert(buddy->mBodyHealth==1000&&buddy->mZombiePhase==0&&w.mZombies.mSize==2);
  assert(SandboxZombies::Damage(z,80)==80&&!SandboxZombies::DetachedArmor(z));
 }
 {World w;auto* z=w.enemy(500,2,static_cast<ZombieType>(3));z->mZombiePhase=PHASE_POLEVAULTER_POST_VAULT;
  SandboxZombies::PoleLanded(z);w.step(1500);assert(z->mZombiePhase==PHASE_POLEVAULTER_POST_VAULT&&!z->mHasObject);}
 {World w;auto* z=w.enemy(500,2,static_cast<ZombieType>(5));z->mZombiePhase=PHASE_NEWSPAPER_MAD;z->mShieldHealth=0;
  SandboxZombies::RecoverPhone(z);w.step(1500);assert(z->mShieldHealth==0&&z->mZombiePhase==PHASE_NEWSPAPER_MAD);}
 for(int phase=SandboxZombies::Held;phase<=SandboxZombies::AirDrop;++phase){World w;auto* z=w.enemy(444,2,static_cast<ZombieType>(21));
  z->mZombiePhase=phase;z->mAltitude=35;z->mPosY=150;z->mTargetPlantID=321;z->mBodyHealth=200;z->mHelmHealth=60;z->mShieldHealth=90;
  SandboxZombies::RestoreNative(&w);assert(z->mZombiePhase==PHASE_LADDER_CARRYING&&z->mPosX==444&&z->mRow==2&&z->mPosY==200&&z->mAltitude==0);
  assert(z->mBodyHealth==200&&z->mHelmHealth==60&&z->mShieldHealth==90&&z->mTargetPlantID==0&&!SandboxZombies::HasInteraction(z));
 }
 for(int style:{19,290,291,292,294,295,512,521}){World w;auto* shot=w.AddProjectile(100,250,0,2,PROJECTILE_PEA);
  assert(!MemeCharacters::RestoreShotStyle(shot,style));assert(!MemeCharacters::ShotStyle(shot));}
 std::cout<<"Meme powers: three characters, native roster and migration passed\\n";
}
