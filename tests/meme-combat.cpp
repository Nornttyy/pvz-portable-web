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
 static_assert(MemeShooterRules::BurstCount==50&&MemeShooterRules::MaxRage==300&&MemeShooterRules::PerShot==20&&MemeShooterRules::BurstTicks==300);
 static_assert(MemeShooterRules::RecoveryDelay==300);
 for(int roll=0;roll<=60;++roll)assert(std::abs(MemeShooterRules::BurstSpeed(roll)-(4.6f+roll*0.05f)*0.75f)<0.0001f);
 {World w;auto* p=w.add(500);w.enemy();w.step(2450);const int shots=w.mProjectiles.mSize;assert(shots==65);w.step(299);assert(w.mProjectiles.mSize==shots);w.step();assert(w.mProjectiles.mSize==shots+1&&MemeCharacters::Data(p,1)==20);}
 {World w;auto* p=w.add(517);p->Fire(nullptr,2,WEAPON_PRIMARY);auto* s=w.mProjectiles.values.back();assert(MemeCharacters::ShotStyle(s)==290);s->mPosX=741;s->mVelY=0.02f;s->mMotionType=MOTION_THREEPEATER;
  w.mPaused=true;SandboxPlants::UpdateShot(s);assert(s->mMotionType==MOTION_THREEPEATER);w.mPaused=false;SandboxPlants::UpdateShot(s);assert(s->mMotionType==MOTION_STAR&&s->mVelX==-3.33f&&s->mVelY==0);
  s->mProjectileType=PROJECTILE_FIREBALL;const int saved=SandboxPlants::SaveShot(s);SandboxPlants::ForgetShot(s);SandboxPlants::RestoreShot(s,saved);assert(MemeCharacters::ShotStyle(s)==290&&MemeCharacters::CanHit(s)&&s->mVelX<0);
  SandboxPlants::UpdateShot(s);assert(s->mVelX==-3.33f);}
 {World w;auto* p=w.add(518);p->mPlantHealth=123;const int x=p->mX,y=p->mY;
  for(int jump=0;jump<2;++jump){p->mState=STATE_SQUASH_DONE_FALLING;p->mTargetX=x+100;p->mX=x+100;p->mY=y+8;
   for(int count=100;count>=0;--count){p->mStateCountdown=count;assert(MemeCharacters::ReturnSquash(p));if(count==35){assert(p->mX==x+50&&p->mY<y-80);const auto saved=SandboxPlants::SavePower(p);SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,saved));}}
   assert(p->mX==x&&p->mY==y&&p->mState==STATE_NOTREADY&&p->mPlantHealth==123&&MemeCharacters::Data(p,4)==2-jump);}
  p->mState=STATE_SQUASH_DONE_FALLING;p->mStateCountdown=0;assert(!MemeCharacters::ReturnSquash(p));}
 {World w;auto* bucket=w.enemy(500,2,static_cast<ZombieType>(4));auto* victim=w.enemy(450);bucket->mHelmHealth=100;bucket->mZombieAge=600;w.step();assert(victim->mBodyHealth==1000&&SandboxZombies::IsResting(bucket)&&SandboxZombies::Speed(bucket)==0);bucket->mZombieAge=620;bucket->mPhaseCounter=20;w.step();assert(victim->mBodyHealth==960);
  bucket->mPhaseCounter=0;bucket->mZombieAge=1200;bucket->mHelmHealth=0;w.step();assert(victim->mBodyHealth==960);}
 {World w;auto* z=w.enemy(500,2,static_cast<ZombieType>(3));z->mZombiePhase=PHASE_POLEVAULTER_POST_VAULT;SandboxZombies::PoleLanded(z);assert(z->mPhaseCounter==800);w.step();assert(z->mZombiePhase==PHASE_POLEVAULTER_POST_VAULT);
  z->mPhaseCounter=0;z->mButteredCounter=100;w.step();assert(!z->mHasObject);z->mButteredCounter=0;w.step();assert(z->mHasObject&&z->mZombiePhase==PHASE_POLEVAULTER_PRE_VAULT&&z->mZombieAttackRect.mX==-29);}
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
 for(int offset:{-5,-2,0,2}){World w;auto* p=w.add(503);p->drawHeightOffset=offset;
  assert(SandboxPlants::RestorePower(p,{503,300,2,0,180,1,0,0,3,1}));w.step();
  assert(w.mProjectiles.mSize==1&&w.mProjectiles.values[0]->mPosY==p->mY+offset+22);
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
 {World w;auto* p=w.add(505);auto* z=w.enemy();w.step(50);assert(w.mProjectiles.mSize==1);auto* s=w.mProjectiles.values[0];assert(s->mProjectileType==PROJECTILE_SNOWPEA&&MemeCharacters::ShotStyle(s)==19);
  const float x=z->mPosX;assert(!SandboxPlants::Impact(s,z)&&z->mPosX==x+40);w.step(299);assert(w.mProjectiles.mSize==1);w.step();assert(w.mProjectiles.mSize==2);
  auto* heavy=w.enemy(300,2,ZOMBIE_GARGANTUAR);SandboxPlants::Impact(s,heavy);assert(heavy->mPosX==300);
  z->mMindControlled=true;SandboxPlants::Impact(s,z);assert(z->mPosX==x+40);
  s->mProjectileType=PROJECTILE_FIREBALL;z->mMindControlled=false;SandboxPlants::Impact(s,z);assert(z->mPosX==x+40);
 }
 {World w;auto* echo=w.add(506);auto* source=w.plant(0,2);auto* echo2=w.add(506,1);w.step(100);assert(w.mProjectiles.mSize==0);
  source->Fire(nullptr,2,WEAPON_PRIMARY);assert(MemeCharacters::Data(echo,4)==1&&MemeCharacters::Data(echo2,4)==0);w.step();assert(w.mProjectiles.mSize==2);
  w.step(300);assert(w.mProjectiles.mSize==2); // Echoes cannot create a loop.
  for(int i=0;i<20;++i)source->Fire(nullptr,2,WEAPON_PRIMARY);assert(MemeCharacters::Data(echo,4)==6);
  auto saved=SandboxPlants::SavePower(echo);SandboxPlants::Forget(echo);assert(SandboxPlants::RestorePower(echo,saved));
  const int before=w.mProjectiles.mSize;w.step(180);assert(w.mProjectiles.mSize==before+6);
 }
 {World w;auto* bucket=w.enemy(500,2,static_cast<ZombieType>(4));bucket->mHelmHealth=1000;bucket->mIsEating=true;
  const float x=bucket->mPosX;
  for(int damage:{20,40,80})assert(SandboxZombies::Damage(bucket,damage)==damage&&bucket->mPhaseCounter==0&&bucket->mPosX==x&&bucket->mIsEating);
  for(int oldCounter:{0,120,180}){bucket->mPhaseCounter=oldCounter;SandboxZombies::Damage(bucket,20);
   assert(bucket->mPhaseCounter==oldCounter&&bucket->mPosX==x&&bucket->mIsEating&&SandboxZombies::Speed(bucket)==1&&!SandboxZombies::IsRetreating(bucket));
  }
 }
 {World w;auto* normal=w.enemy(450),*flag=w.enemy(550,2,static_cast<ZombieType>(1));assert(SandboxZombies::Speed(normal)==1.5f);
  w.enemy(520,2,static_cast<ZombieType>(1));assert(SandboxZombies::Speed(normal)==1.5f);flag->mDead=true;
  auto* giant=w.enemy(450,2,ZOMBIE_GARGANTUAR);assert(SandboxZombies::Speed(giant)==1);
  gSandboxEnabled=false;app.adventure=false;assert(SandboxZombies::Speed(normal)==1);gSandboxEnabled=true;app.adventure=true;
 }
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
 // The free showoff shroom lasts exactly 60 simulation seconds in both modes.
 // Each planting owns its deadline; native puff-shrooms and supports survive.
 static_assert(MemeCharacters::ShowoffLifetime==6000);
 for(bool sandbox:{false,true}){World w;gSandboxEnabled=sandbox;
  auto* native=w.plant(0,2);native->mSeedType=static_cast<SeedType>(8);
  auto* support=w.plant(1,2);support->mSeedType=static_cast<SeedType>(16);
  auto* first=w.add(502);assert(MemeCharacters::Data(first,5)==6000);
  w.step(1000);auto* later=w.add(502,3);w.step(4999);
  assert(!first->mDead&&!later->mDead&&MemeCharacters::Data(first,5)==1);
  w.step();assert(first->mDead&&!MemeCharacters::Is(first)&&!later->mDead);
  assert(MemeCharacters::Data(later,5)==1000&&!native->mDead&&!support->mDead);
  w.step(999);assert(!later->mDead);w.step();assert(later->mDead);
 }gSandboxEnabled=true;
 {World w;auto* p=w.add(502);w.step(2500);const auto saved=SandboxPlants::SavePower(p);
  w.mPaused=true;w.step(10000);assert(SandboxPlants::SavePower(p)==saved&&!p->mDead);
  SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,saved));
  assert(MemeCharacters::Data(p,5)==3500);w.step(1000);assert(SandboxPlants::SavePower(p)==saved);
  w.mPaused=false;w.step(3499);assert(!p->mDead);w.step();assert(p->mDead);
 }
 {World w;auto* p=w.add(502);p->SetSleeping(true);w.step(5999);
  assert(!p->mDead&&p->mIsAsleep&&MemeCharacters::Data(p,5)==1);w.step();assert(p->mDead);
 }
 // Older unlimited-life saves still load, then expire on a live tick if due.
 for(int age:{5999,6000,50000}){World w;auto* p=w.add(502);auto state=SandboxPlants::SavePower(p);state[6]=age;
  assert(SandboxPlants::RestorePower(p,state));w.mPaused=true;w.step(10);assert(!p->mDead);
  w.mPaused=false;w.step();assert(p->mDead);
 }
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
 {World w;auto* p=w.add(507);p->mPlantHealth=137;
  for(int lives=3;lives>0;--lives){
   assert(MemeCharacters::Data(p,4)==lives);float x=0,y=0,sx=1,sy=1;MemeCharacters::Scale(p,x,y,sx,sy);
   assert(std::abs(sx-(0.6f+0.2f*(lives-1)))<0.001f&&sx==sy&&std::abs(x+40*sx-40)<0.001f&&std::abs(y+65*sy-65)<0.001f);
   auto save=SandboxPlants::SavePower(p);SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,save));
   assert(MemeCharacters::RearmPotato(p)==(lives>1));assert(p->mPlantHealth==137&&!p->mDead);
  }
  auto invalid=SandboxPlants::SavePower(p);invalid[8]=0;assert(!SandboxPlants::RestorePower(p,invalid));
  auto* native=w.plant(2,2);assert(!MemeCharacters::RearmPotato(native));p->mDead=true;assert(!MemeCharacters::RearmPotato(p));
 }
 {World w;auto* cone=w.enemy(400,2,static_cast<ZombieType>(2));cone->mHelmHealth=20;
  SandboxZombies::ArmorBroken(cone);assert(!SandboxZombies::IsFeigning(cone));
  cone->mHelmHealth=0;cone->mIsEating=true;const int hp=cone->mBodyHealth;SandboxZombies::ArmorBroken(cone);
  assert(SandboxZombies::IsFeigning(cone)&&cone->mPhaseCounter==300&&!cone->mIsEating&&SandboxZombies::Speed(cone)==0&&cone->mBodyHealth==hp);
  cone->mPhaseCounter=240;assert(SandboxZombies::Damage(cone,20)==20&&cone->mPhaseCounter==240);cone->TakeDamage(20,0);assert(cone->mBodyHealth==hp-20);
  Reanimation body;body.mOverlayMatrix.m02=15;body.mOverlayMatrix.m12=20;SandboxZombies::AdjustPose(cone,&body);const auto& m=body.mOverlayMatrix;
  assert(m.m01>0.9f&&m.m10<-0.9f&&std::abs(m.m00*45+m.m01*120+m.m02-60)<0.001f&&std::abs(m.m10*45+m.m11*120+m.m12-140)<0.001f);
  for(int timer:{300,1,0}){cone->mPhaseCounter=timer;body.mOverlayMatrix={};SandboxZombies::AdjustPose(cone,&body);assert(std::abs(body.mOverlayMatrix.m01)<0.01f);}
  assert(!SandboxZombies::IsFeigning(cone)&&SandboxZombies::Speed(cone)==1&&cone->mBodyHealth==hp-20);
  cone->mPhaseCounter=200;cone->mMindControlled=true;assert(!SandboxZombies::IsFeigning(cone));cone->mMindControlled=false;cone->mHasHead=false;assert(!SandboxZombies::IsFeigning(cone));
  auto* normal=w.enemy();SandboxZombies::ArmorBroken(normal);assert(normal->mPhaseCounter==0);
  cone->mHasHead=true;gSandboxEnabled=false;app.adventure=false;assert(!SandboxZombies::IsFeigning(cone));gSandboxEnabled=true;app.adventure=true;
 }
 // Six added roles keep native animation clocks; only their extra queues are custom.
 for(int id=508;id<=513;++id){World w;auto* p=w.add(id);p->mLaunchCounter=81;p->mShootingCounter=17;auto state=SandboxPlants::SavePower(p);SandboxPlants::Forget(p);
  assert(SandboxPlants::RestorePower(p,state));w.step();assert(p->mLaunchCounter==81&&p->mShootingCounter==17);
 }
 {World w;auto* p=w.add(508);p->mState=STATE_CACTUS_LOW;p->Fire(nullptr,2,WEAPON_SECONDARY);assert(MemeCharacters::Data(p,4)==5);
  w.step(16);assert(w.mProjectiles.mSize==3);auto save=SandboxPlants::SavePower(p);SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,save));w.step(24);
  assert(w.mProjectiles.mSize==6&&MemeCharacters::Data(p,4)==0);for(auto* shot:w.mProjectiles)assert(shot->mProjectileType==PROJECTILE_SPIKE);
  w.step(350);assert(w.mProjectiles.mSize==6); // No native first shot is simulated by this state-only fixture.
  p->mState=STATE_CACTUS_HIGH;p->Fire(nullptr,2,WEAPON_PRIMARY);w.step(40);assert(w.mProjectiles.mSize==12);
 }
 {World w;auto* p=w.add(509);p->mState=STATE_CHOMPER_DIGESTING;p->mStateCountdown=4000;p->mPlantHealth=111;
  auto* neighbor=w.plant(2,2);neighbor->Fire(nullptr,2,WEAPON_PRIMARY);assert(p->mStateCountdown==3800&&p->mPlantHealth==111);
  for(int i=0;i<80;++i)neighbor->Fire(nullptr,2,WEAPON_PRIMARY);assert(p->mStateCountdown==3800);w.step(50);neighbor->Fire(nullptr,2,WEAPON_PRIMARY);assert(p->mStateCountdown==3600);
  auto* far=w.plant(4,4);w.step(50);far->Fire(nullptr,4,WEAPON_PRIMARY);assert(p->mStateCountdown==3600);
  p->mStateCountdown=100;neighbor->Fire(nullptr,2,WEAPON_PRIMARY);assert(p->mStateCountdown==0&&p->mPlantHealth==111);
 }
 {World w;auto* p=w.add(510);auto* front=w.enemy(200),*last=w.enemy(650);auto* wrongLane=w.enemy(750,1),*preview=w.enemy(900);assert(MemeCharacters::PickTarget(p,front)==last);
  last->mMindControlled=true;assert(MemeCharacters::PickTarget(p,front)==front);last->mMindControlled=false;last->mDead=true;assert(MemeCharacters::PickTarget(p,front)==front);
  assert(MemeCharacters::PickTarget(p,nullptr)==nullptr);assert(MemeCharacters::PickTarget(w.plant(2,2),front)==front);
 }
 {World w;auto* p=w.add(511);assert(!MemeCharacters::StarTarget(p));w.enemy(250,0);assert(MemeCharacters::StarTarget(p));
  float lastX=0,lastY=0;for(int volley=0;volley<9;++volley){for(int i=0;i<5;++i){auto* s=w.AddProjectile(100,200,0,2,PROJECTILE_STAR);s->mMotionType=MOTION_STAR;s->mVelX=3.33f;s->mVelY=0;MemeCharacters::OnFired(p,s);
   assert(std::abs(std::hypot(s->mVelX,s->mVelY)-3.33f)<0.001f);if(i==0){if(volley>0)assert(std::abs(s->mVelX-lastX)+std::abs(s->mVelY-lastY)>0.5f);lastX=s->mVelX;lastY=s->mVelY;}}
   assert(MemeCharacters::Data(p,4)==0);auto save=SandboxPlants::SavePower(p);assert(SandboxPlants::RestorePower(p,save));
  }
 }
 {World w;auto* p=w.add(512);auto* z=w.enemy();assert(!MemeCharacters::ButterReady(p));p->Fire(z,2,WEAPON_PRIMARY);p->Fire(z,2,WEAPON_PRIMARY);assert(MemeCharacters::ButterReady(p));
  p->Fire(z,2,WEAPON_SECONDARY);assert(!MemeCharacters::ButterReady(p)&&MemeCharacters::Data(p,4)==5);w.step(130);assert(w.mProjectiles.mSize==3);z->mButteredCounter=200;
  w.step(60);assert(w.mProjectiles.mSize==8&&MemeCharacters::Data(p,4)==0&&!MemeCharacters::ButterReady(p));
  p->Fire(z,2,WEAPON_SECONDARY);z->mButteredCounter=0;w.step(400);assert(w.mProjectiles.mSize==9&&MemeCharacters::Data(p,4)==0);
 }
 {World w;auto* p=w.add(513);p->mState=STATE_SCAREDYSHROOM_SCARED;w.step(1000);assert(MemeCharacters::Data(p,4)==6&&w.mProjectiles.mSize==0);
  p->mState=STATE_READY;w.enemy();w.step(61);assert(w.mProjectiles.mSize==6&&MemeCharacters::Data(p,4)==0);for(auto* s:w.mProjectiles)assert(s->mProjectileType==PROJECTILE_PUFF);
  p->mState=STATE_SCAREDYSHROOM_SCARED;p->mIsAsleep=true;w.step(500);assert(MemeCharacters::Data(p,4)==0);
 }
 {World w;auto* z=w.enemy();for(int age:{0,599,800}){z->mZombieAge=age;assert(!SandboxZombies::IsResting(z));}for(int age:{600,650,799}){z->mZombieAge=age;z->mIsEating=true;w.step();assert(SandboxZombies::IsResting(z)&&SandboxZombies::Speed(z)==0&&!z->mIsEating);}
  auto* football=w.enemy(600,2,static_cast<ZombieType>(7));football->mZombieAge=100;assert(SandboxZombies::Speed(football)==1.8f);football->mZombieAge=200;assert(SandboxZombies::IsResting(football));football->mZombieAge=350;assert(SandboxZombies::Speed(football)==1);
  football->mZombieAge=250;football->mMindControlled=true;assert(!SandboxZombies::IsResting(football));
 }
 {World w;auto* victim=w.enemy(500),*guard=w.enemy(510,1,static_cast<ZombieType>(6));guard->mShieldHealth=100;
  assert(SandboxZombies::Damage(victim,20)==10&&guard->mShieldHealth==90&&guard->mPhaseCounter==20);assert(SandboxZombies::Damage(victim,40,1)==40&&guard->mShieldHealth==90);
  assert(SandboxZombies::Damage(victim,1800)==1800);guard->mShieldHealth=3;assert(SandboxZombies::Damage(victim,20)==17&&guard->mShieldHealth==0);assert(SandboxZombies::Damage(victim,20)==20);
  guard->mShieldHealth=100;guard->mMindControlled=true;assert(SandboxZombies::Damage(victim,20)==20);guard->mMindControlled=false;guard->mPosX=650;assert(SandboxZombies::Damage(victim,20)==20);
  guard->mPosX=510;assert(SandboxZombies::Damage(guard,20)==20);
 }
 {World w;auto* balloon=w.enemy(600,2,static_cast<ZombieType>(16));balloon->flying=true;balloon->mZombieAge=599;w.step();assert(w.mZombies.mSize==1);balloon->mZombieAge=600;w.step();assert(w.mZombies.mSize==2);
  auto* drop=w.mZombies.values[1];assert(drop->mPosX==625&&drop->mRow==2&&drop->mAltitude==80&&drop->mZombieHeight==HEIGHT_FALLING);balloon->mZombieAge=601;w.step(1000);assert(w.mZombies.mSize==2);
 }
 {World w;w.pool=true;auto* balloon=w.enemy(600,2,static_cast<ZombieType>(16));balloon->flying=true;balloon->mZombieAge=600;w.step();assert(w.mZombies.mSize==1);}
 {World w;auto* p=w.add(514);w.step(500);assert(MemeCharacters::Data(p,1)==0);
  auto* z=w.enemy(p->mX+130);w.step(299);assert(z->mBodyHealth==1000&&MemeCharacters::Data(p,1)==299);
  const auto saved=SandboxPlants::SavePower(p);w.mPaused=true;w.step(1000);assert(SandboxPlants::SavePower(p)==saved);w.mPaused=false;
  assert(SandboxPlants::RestorePower(p,saved));w.step();assert(z->mBodyHealth==980);w.step(130);assert(z->mBodyHealth==940&&MemeCharacters::Data(p,0)==0);
  p->mIsAsleep=true;w.step(600);assert(z->mBodyHealth==940);p->mIsAsleep=false;
  p->mShootingCounter=17;const auto mid=SandboxPlants::SavePower(p);SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,mid)&&p->mShootingCounter==17);
 }
 {World w;auto* p=w.add(515);auto* z=w.enemy();const auto sounds=app.memeCues.size();w.step(50);assert(w.mProjectiles.mSize==1&&MemeCharacters::Data(p,4)==5);
  w.step(28);auto state=SandboxPlants::SavePower(p);assert(state[8]==3);SandboxPlants::Forget(p);assert(SandboxPlants::RestorePower(p,state));w.step(42);
  assert(w.mProjectiles.mSize==6&&app.memeCues.size()==sounds+2);
  for(int i=0;i<6;++i)assert((w.mProjectiles.values[i]->mMotionType==MOTION_BACKWARDS)==bool(i%2));
  const float x=z->mPosX;w.step(249);assert(w.mProjectiles.mSize==6&&z->mPosX==x);w.step();assert(w.mProjectiles.mSize==7);
 }
 {World w;auto* p=w.add(515);w.enemy(p->mX-50);w.step(50);assert(w.mProjectiles.mSize==1&&w.mProjectiles.values[0]->mMotionType==MOTION_BACKWARDS);}
 {World w;auto* p=w.add(516);auto* z=w.enemy(p->mX-10);auto* giant=w.enemy(p->mX-10,2,ZOMBIE_GARGANTUAR);const float x=z->mPosX;
  w.step(50);assert(z->mBodyHealth==980&&z->mAltitude==24&&z->mZombieHeight==HEIGHT_FALLING&&z->mPosX==x&&giant->mBodyHealth==1000);
  z->mZombieHeight=HEIGHT_ZOMBIE_NORMAL;z->mAltitude=0;w.step(199);assert(z->mBodyHealth==980);w.step();assert(z->mBodyHealth==960);
 }
 {World w;auto* p=w.add(516);auto* z=w.enemy(p->mX-10);z->mIceTrapCounter=100;w.step(200);assert(z->mBodyHealth==1000);z->mIceTrapCounter=0;z->mInPool=true;w.step();assert(z->mBodyHealth==1000);}
 {World w;auto* imp=w.enemy(500,2,ZOMBIE_IMP);imp->mZombieAge=400;imp->mIsEating=true;assert(SandboxZombies::IsRetreating(imp)&&SandboxZombies::Speed(imp)<0);w.step();assert(!imp->mIsEating);
  imp->mZombieHeight=HEIGHT_FALLING;assert(!SandboxZombies::IsRetreating(imp));imp->mZombieHeight=HEIGHT_ZOMBIE_NORMAL;
  imp->mMindControlled=true;assert(!SandboxZombies::IsRetreating(imp));imp->mMindControlled=false;
  imp->mZombieAge=500;assert(SandboxZombies::Speed(imp)==1);imp->mZombieAge=1000;assert(SandboxZombies::IsRetreating(imp));
 }
 {World w;auto* z=w.enemy(400,2,static_cast<ZombieType>(21));z->mShieldHealth=500;z->mZombiePhase=PHASE_LADDER_CARRYING;z->mZombieAge=600;w.plant(4,2);
  const float x=z->mPosX;w.step();assert(z->mRow==1&&z->mPosX==x&&z->mShieldHealth==500&&z->mPhaseCounter==35);
 }
 for(int blocked=0;blocked<4;++blocked){World w;auto* z=w.enemy(400,0,static_cast<ZombieType>(21));z->mShieldHealth=500;z->mZombiePhase=PHASE_LADDER_CARRYING;z->mZombieAge=600;w.plant(4,0);
  if(blocked==0)z->mShieldHealth=0;if(blocked==1)z->mZombiePhase=PHASE_LADDER_PLACING;if(blocked==2)z->mMindControlled=true;if(blocked==3)z->mIceTrapCounter=100;
  w.step();assert(z->mRow==0);
 }
 {World w;w.pool=true;auto* z=w.enemy(400,1,static_cast<ZombieType>(21));z->mShieldHealth=500;z->mZombiePhase=PHASE_LADDER_CARRYING;z->mZombieAge=600;w.plant(4,1);w.plant(4,0);w.step();assert(z->mRow==1);}
 for(int id=500;id<=516;++id){World w;auto* p=w.add(id);w.step(100);auto state=SandboxPlants::SavePower(p);
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
