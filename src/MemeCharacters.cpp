// Individual character mechanics using the native rigs and seed bank.
#include "MemeCharacters.h"
#include "MemeShooterRules.h"
#include "SandboxPlants.h"
#include "SandboxArt.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "Lawn/Projectile.h"
#include "Lawn/SeedPacket.h"
#include "PvzpLib/Reanimator.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"
#include <map>
#include <algorithm>
#include <cmath>
#include <string>
namespace MemeCharacters {
namespace {
// Saved in the existing ten-integer optional plant record.
struct State {int id=0,health=0,phase=0,heat=0,timer=0,delay=50,age=0,pulse=0,remaining=0,direction=1;};
std::map<const Plant*,State> states;
std::map<const Projectile*,int> shotStyles; // 1..8 wobble, 9 scatter; native art/motion/save.
void Burst(State& s){
 s.phase=1;s.heat=0;s.remaining=MemeShooterRules::BurstCount;s.delay=0;s.timer=0;s.pulse=30;
 gLawnApp->PlayRageRelease(); // Once per release, never once per pea or save restore.
}
// Native zombie IDs are stable through DataArray recycling; never retain pointers.
std::map<unsigned,int> laneCooldown;
bool Enemy(Zombie* z){return !z->mDead&&z->IsOnBoard()&&!z->mMindControlled&&!z->IsDeadOrDying()&&z->mHasHead;}
bool Walker(Zombie* z){const int type=int(z->mZombieType);return Enemy(z)&&(type==0||type==1||type==2||type==4||type==5||type==6||type==7||type==24)&&z->mZombiePhase==PHASE_ZOMBIE_NORMAL&&z->mZombieHeight==HEIGHT_ZOMBIE_NORMAL&&!z->mInPool;}
bool Lane(Board* b,int from,int to){return to>=0&&to<(b->StageHasPool()?6:5)&&b->RowCanHaveZombies(to)&&!(b->StageHasPool()&&(from==2||from==3||to==2||to==3));}
bool ReadyToMove(Board* b,Zombie* z){return Walker(z)&&!laneCooldown.contains(unsigned(b->ZombieGetID(z)));}
void Move(Board* b,Zombie* z,int row){
 z->SetRow(row); // Native UpdateZombiePosition smoothly walks to the new lane.
 laneCooldown[unsigned(b->ZombieGetID(z))]=350;
}
void Shoot(Plant* p,Zombie* target){
 p->Fire(target,p->mRow,WEAPON_PRIMARY);
 if(auto* anim=gLawnApp->ReanimationTryToGet(p->mHeadReanimID);anim&&anim->TrackExists("anim_shooting"))anim->PlayReanim("anim_shooting",REANIM_PLAY_ONCE_AND_HOLD,3,35.0f);
}
void Puff(Sexy::Graphics* g,float x,float y,int age,int alpha){
 auto* im=SandboxArt::NativeImage(age<15?"puff_3.png":"puff_4.png");if(!im)return;
 Sexy::SexyTransform2D m;m.LoadIdentity();m.m00=(18+age*0.6f)/im->mWidth;m.m11=(18+age*0.6f)/im->mHeight;m.m02=x+g->mTransX;m.m12=y+g->mTransY;
 PvzpBltMatrix(g,im,m,g->mClipRect,Sexy::Color(255,255,255,alpha),g->mDrawMode,Sexy::Rect(0,0,im->mWidth,im->mHeight));
}
}
int Type(const Plant* p){auto it=states.find(p);return it==states.end()?0:it->second.id;}
bool Is(const Plant* p){return states.contains(p);}
bool Hiding(const Plant* p){auto it=states.find(p);return it!=states.end()&&(it->second.id==503||it->second.id==504)&&it->second.phase==1&&!p->mIsAsleep&&!p->mSquished;}
bool Producing(const Plant* p){auto it=states.find(p);return it!=states.end()&&it->second.id==503&&it->second.phase==0;}
void Reset(){states.clear();laneCooldown.clear();shotStyles.clear();}
void Forget(Plant* p){states.erase(p);}
void Assign(Plant* p,int id){const auto* d=Find(id);if(!d||int(p->mSeedType)!=d->base)return;
 State s;s.id=id;s.health=p->mPlantHealth;s.timer=id==502?180:0;if(id==500)s.direction=2;states[p]=s;
 p->mLaunchCounter=id==503?std::clamp(p->mLaunchCounter,300,2500):9999;p->mShootingCounter=0;
 if(id==502)p->SetSleeping(false); // Keep the native short-range shot and add a daytime lure.
}
std::array<int,10> Save(const Plant* p){auto it=states.find(p);if(it==states.end())return {};const auto& s=it->second;return {s.id,s.health,s.phase,s.heat,s.timer,s.delay,s.age,s.pulse,s.remaining,s.direction};}
bool Restore(Plant* p,const std::array<int,10>& a){
 const bool shooter=a[0]==500,newShooter=shooter&&a[9]==2;
 const auto* d=Find(a[0]);if(!d||int(p->mSeedType)!=d->base||p->mDead||a[1]<0||a[1]>p->mPlantMaxHealth||a[2]<0||a[2]>(newShooter?1:2)||a[3]<0||a[3]>(newShooter?300:1000)||a[4]<0||a[4]>2000||a[5]<0||a[5]>2000||a[6]<0||a[6]>=1000000||a[7]<0||a[7]>50||a[8]<0||a[8]>(newShooter?50:3)||(!newShooter&&a[9]!=1&&a[9]!=-1))return false;
 if(newShooter&&((a[2]==1&&(a[8]==0||a[3]!=0))||(a[2]==0&&a[8]!=0)))return false;
 State s{a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9]};
 // Direction was unused for this character: 2 versions the new burst state.
 // Migrate old overheating saves without healing or inventing a free volley.
 if(shooter&&!newShooter){s.phase=0;s.heat=a[2]==0?a[3]*300/1000:0;s.timer=0;s.delay=std::min(a[5],150);s.remaining=0;s.pulse=0;s.direction=2;}
 states[p]=s;p->mLaunchCounter=a[0]==503?std::clamp(p->mLaunchCounter,0,2500):9999;p->mShootingCounter=0;return true;
}
int Data(const Plant* p,int field){const auto it=states.find(p);if(it==states.end())return -1;const auto& s=it->second;return field==0?s.phase:field==1?s.heat:field==2?s.timer:field==3?s.direction:s.remaining;}
bool Activate(Plant* p,int direction){
 auto it=states.find(p);if(it==states.end()||p->mDead||p->mBoard->mPaused||p->mIsAsleep||p->mSquished||p->NotOnGround()||p->mPlantHealth<=0)return false;
 auto& s=it->second;
 if(s.id==500){
  if(s.phase!=0||s.heat<MemeShooterRules::ManualRage)return false;
  Burst(s);return true;
 }
 return false;
}
bool Click(Board* b,int x,int y){
 for(auto* p:b->mPlants)if(!p->mDead&&Is(p)&&x>=p->mX&&x<p->mX+80&&y>=p->mY&&y<p->mY+80){
  if(Type(p)==500){Activate(p,y<p->mY+40?-1:1);return true;}
 }return false;
}
void Tick(Board* b){
 if(b->mPaused)return;
 for(auto it=laneCooldown.begin();it!=laneCooldown.end();)if(--it->second<=0||!b->ZombieTryToGet(static_cast<ZombieID>(it->first)))it=laneCooldown.erase(it);else ++it;
 for(auto* p:b->mPlants){
  auto it=states.find(p);if(it==states.end())continue;if(p->mDead){states.erase(it);continue;}auto& s=it->second;
  if(s.id!=503)p->mLaunchCounter=9999;p->mShootingCounter=0;
  if(p->mIsAsleep||p->mSquished||p->NotOnGround()||p->mPlantHealth<=0)continue;
  s.age=(s.age+1)%1000000;if(s.pulse)--s.pulse;if(s.delay)--s.delay;if(s.timer)--s.timer;
  if(s.id==500){
   if(s.phase==0&&s.heat>=MemeShooterRules::MaxRage)Burst(s);
   const bool room=b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8;
   if(s.phase==1){
    if(!s.delay&&room){
     Shoot(p,nullptr);--s.remaining;s.delay=MemeShooterRules::BurstDelay;s.pulse=10;
     if(!s.remaining){s.phase=0;s.heat=0;s.delay=MemeShooterRules::NormalDelay;}
    }
   }else if(!s.delay&&room){
    if(auto* target=p->FindTargetZombie(p->mRow,WEAPON_PRIMARY)){
     Shoot(p,target);s.heat+=MemeShooterRules::PerShot;s.delay=MemeShooterRules::NormalDelay;
     if(s.heat>=MemeShooterRules::MaxRage)Burst(s);
    }
   }
  }else if(s.id==501){
   if(!s.timer&&p->mRecentlyEatenCountdown>0){
    Zombie* target=nullptr;
    for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mIsEating&&z->mPosX>=p->mX-65&&z->mPosX<p->mX+35&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))&&(!target||z->mPosX>target->mPosX))target=z;
    if(target){s.timer=200;s.pulse=50;s.phase=1;s.heat=int(target->mPosX-p->mX+65);gLawnApp->PlayFoley(FOLEY_THROW);}
   }
   if(s.phase==1&&s.pulse==28){
    for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mPosX>=p->mX-65&&z->mPosX<p->mX+40&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))){
     const bool push=Walker(z);z->TakeDamage(80,0);
     if(push&&!z->IsDeadOrDying()){z->StopEating();z->mPosX=std::min(850.0f,z->mPosX+24);z->UpdateReanim();}break;
    }
   }
   if(!s.pulse)s.phase=0;
  }else if(s.id==502){
   if(!s.delay&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8)if(auto* target=p->FindTargetZombie(p->mRow,WEAPON_PRIMARY)){Shoot(p,target);s.delay=150;}
   s.heat=(500-std::min(500,s.timer))*2;
   if(!s.timer){
    Zombie* closest=nullptr;float distance=10000;
    for(auto* z:b->mZombies)if(ReadyToMove(b,z)&&std::abs(z->mRow-p->mRow)==1&&Lane(b,z->mRow,p->mRow)&&z->mPosX>p->mX+20&&z->mPosX<p->mX+310){const float d=std::abs(z->mPosX-p->mX);if(d<distance){closest=z;distance=d;}}
    if(closest){Move(b,closest,p->mRow);s.timer=500;s.pulse=50;gLawnApp->PlayFoley(FOLEY_THROW);}else s.timer=30;
   }
  }else if(s.id==503){
   Zombie *front=nullptr,*back=nullptr;
   for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))){
    // Normal bite box is x+50..70. Hide just before contact and wake
    // after the whole bite box passes, not a full extra tile behind.
    if(z->mPosX>=p->mX-65&&z->mPosX<p->mX+45)front=z;
    if(z->mPosX<p->mX-65&&z->mPosX>p->mX-330&&(!back||z->mPosX>back->mPosX))back=z;
   }
   if(s.phase==0&&!s.timer&&front){s.phase=1;s.timer=1400;s.pulse=20;}
   if(s.phase==1&&(!front||!s.timer)){
    // A killed, charmed or diverted enemy must not leave the flower flat.
    // Reverse even a partially completed collapse without snapping its scale.
    const bool ambush=back&&!front;
    s.pulse=(20-std::clamp(s.pulse,0,20))*3/2;
    s.phase=2;s.timer=ambush?180:std::max(1,s.pulse);
    s.remaining=ambush?3:0;s.delay=ambush?35:0;
   }
   if(s.phase==2){
    if(s.remaining&&!s.delay&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8){
     auto* shot=b->AddProjectile(p->mX+12,p->mY+22,p->mRenderOrder-1,p->mRow,PROJECTILE_PEA);
     shot->mMotionType=MOTION_BACKWARDS;shot->mDamageRangeFlags=p->GetDamageRangeFlags(WEAPON_PRIMARY);SandboxPlants::RestoreShot(shot,300);
     --s.remaining;s.delay=16;gLawnApp->PlayFoley(FOLEY_THROW);
    }
    if(!s.timer){s.phase=0;s.timer=500;s.remaining=0;s.pulse=0;}
   }
   s.heat=s.phase==1?1000:s.phase==2?std::min(1000,s.timer*5):std::max(0,1000-s.timer*2);
  }else if(s.id==504){
   if(!s.phase&&!s.timer){
    Zombie* target=nullptr;
    for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mPosX>=p->mX-25&&z->mPosX<p->mX+260&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))&&(!target||z->mPosX<target->mPosX))target=z;
    if(target){s.phase=1;s.timer=80;s.heat=std::clamp(int(target->mPosX+20-p->mX),20,280);gLawnApp->PlayFoley(FOLEY_THROW);}
   }else if(s.phase==1){
    if(s.timer==40){for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&std::abs(z->mPosX+20-p->mX-s.heat)<60&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY)))z->TakeDamage(80,0);s.pulse=24;}
    if(!s.timer){s.phase=0;s.timer=300;s.heat=0;}
   }
  }
  s.health=p->mPlantHealth;
 }
}
void Tint(const Plant* p,Sexy::Color& c){auto it=states.find(p);if(it==states.end())return;const auto& s=it->second;
 if(s.id==502)c.mBlue=c.mBlue*180/255;
 if(s.id==503){c.mRed=c.mRed*205/255;c.mGreen=c.mGreen*215/255;}
}
void Scale(const Plant* p,float& x,float& y,float& sx,float& sy){auto it=states.find(p);if(it==states.end()||p->mSquished)return;const auto& s=it->second;
 float horizontal=1,vertical=1;
 if(s.id==500&&s.phase==1){const float recoil=std::sin(s.pulse*0.3f)*0.025f;horizontal+=0.04f+recoil;vertical-=0.03f+recoil;}
 if(s.id==501&&s.phase==1&&s.pulse){
  // Keep the original face, damage frames and grid anchor during the bump.
  const float age=50-s.pulse;float offset=0;
  if(age<10){const float q=age/10;offset=-6*q;horizontal=1+0.10f*q;vertical=1-0.08f*q;}
  else if(age<22){const float q=(age-10)/12,e=q*q*(3-2*q);offset=-6+34*e;horizontal=1.10f-0.16f*e;vertical=0.92f+0.12f*e;}
  else{const float q=(age-22)/28,e=q*q*(3-2*q);offset=28*(1-e);horizontal=0.94f+0.06f*e;vertical=1.04f-0.04f*e;}
  x+=std::abs(sx)*offset;
 }
 if(s.id==502&&s.pulse){vertical+=0.12f*std::sin(s.pulse*0.18f);horizontal-=0.06f*std::sin(s.pulse*0.18f);}
 if(s.id==503&&(s.phase==1||s.phase==2)){
  const float crouch=s.phase==1?1-std::clamp(s.pulse/20.0f,0.0f,1.0f):std::clamp(s.pulse/30.0f,0.0f,1.0f);
  vertical=1-0.72f*crouch;horizontal=1+0.15f*crouch;
 }
 if(s.id==504&&s.phase==1){const float t=(80-s.timer)/80.0f,travel=std::sin(3.14159265f*t);x+=std::abs(sx)*s.heat*travel;y-=sy*55*std::abs(std::sin(6.2831853f*t));horizontal=1+0.12f*travel;vertical=1-0.12f*travel;}
 x+=40*sx*(1-horizontal);y+=65*sy*(1-vertical);sx*=horizontal;sy*=vertical;
}
void Effects(Sexy::Graphics* g,Board* b,int row){
 for(const auto& [p,s]:states)if(!p->mDead&&p->mRow==row&&!p->mSquished){
  const int x=p->mX,y=p->mY;
  if(s.id==500){
   const int glow=p->mIsAsleep?0:MemeShooterRules::ReadyGlow(s.heat,s.phase,s.age);
   if(glow){
    g->SetColor(Sexy::Color(210,164,77,glow));
    g->FillRect(x+11,y+76,58,1);g->FillRect(x+11,y+83,58,1);
    g->FillRect(x+11,y+77,1,6);g->FillRect(x+68,y+77,1,6);
   }
   g->SetColor(Sexy::Color(57,37,18));g->FillRect(x+12,y+77,56,6);
   g->SetColor(s.phase==1?Sexy::Color(210,72,43):Sexy::Color(226,167,64));g->FillRect(x+13,y+78,s.phase==1?s.remaining*54/50:s.heat*54/300,4);
   g->SetColor(Sexy::Color(85,57,27));g->FillRect(x+31,y+78,1,4); // 100-rage click threshold, no extra text.
   if(s.phase==1){const int age=s.age%30;Puff(g,x+45,y+15-age,age,130);}
  }else if(s.id==504&&s.phase==1){
   auto* im=Sexy::IMAGE_PROJECTILEPEA;
   if(im){Sexy::SexyTransform2D m;m.LoadIdentity();m.m02=x+40+g->mTransX;m.m12=y+61+g->mTransY;PvzpBltMatrix(g,im,m,g->mClipRect,Sexy::Color(255,255,255),g->mDrawMode,Sexy::Rect(0,0,im->mWidth,im->mHeight));}
   if(s.pulse)Puff(g,x+s.heat+40,y+50,24-s.pulse,140);
  }else if(s.id==502){
   if(s.pulse){const int r=50-s.pulse;for(int sign:{-1,1})Puff(g,x+40,y+25+sign*r,r,130);}
   PvzpDrawString(g,"!",x+40,y+5,Sexy::FONT_BRIANNETOD12,Sexy::Color(248,202,74),DS_ALIGN_CENTER);
  }else if(s.id==503&&s.phase==2&&s.pulse)Puff(g,x+5,y+20,30-s.pulse,120);
 }
}
void Card(Sexy::Graphics* g,int x,int y,int id){const auto* d=Find(id);if(!d)return;
 DrawSeedPacket(g,x,y,static_cast<SeedType>(d->base),SEED_NONE,0,255,false,false);
 Sexy::Graphics bottom(*g);bottom.SetClipRect(x+3,y+52,44,17);PvzpDrawImageCelScaledF(&bottom,Sexy::IMAGE_SEEDS,x,y,2,0,1,1);
 PvzpDrawString(g,std::to_string(d->cost),x+23,y+65,Sexy::FONT_BRIANNETOD12,Sexy::Color(75,51,20),DS_ALIGN_CENTER);
}
void OnFired(Plant* p,Projectile* shot){if(Type(p)==500){
 float x,y;if(SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(p->mHeadReanimID),"idle_mouth",35,49,32,24.5f,x,y)){shot->mPosX=p->mX+x-12;shot->mPosY=p->mY+y-12-shot->mPosZ;shot->mX=int(shot->mPosX);shot->mY=int(shot->mPosY+shot->mPosZ);}
 const auto& s=states.at(p);shot->mMotionType=MOTION_STAR;
 if(s.phase==1){
  shotStyles[shot]=9;const float angle=MemeShooterRules::SpreadAngle(50-s.remaining);
  shot->mVelX=5.1f*std::cos(angle);shot->mVelY=5.1f*std::sin(angle);
 }else{
  const int style=1+(s.heat/20+p->mPlantCol*3+p->mRow)%8;
  shotStyles[shot]=style;shot->mVelX=3.33f;shot->mVelY=MemeShooterRules::WobbleStep(style,0);
 }
}}
int ShotStyle(const Projectile* shot){const auto it=shotStyles.find(shot);return it==shotStyles.end()?0:it->second;}
bool RestoreShotStyle(const Projectile* shot,int style){
 if(style<0||style>9||shot->mDead)return false;
 if(style&&(shot->mMotionType!=MOTION_STAR||(shot->mProjectileType!=PROJECTILE_PEA&&shot->mProjectileType!=PROJECTILE_SNOWPEA&&shot->mProjectileType!=PROJECTILE_FIREBALL)))return false;
 if(style)shotStyles[shot]=style;else shotStyles.erase(shot);return true;
}
void ForgetShot(const Projectile* shot){shotStyles.erase(shot);}
void UpdateShot(Projectile* shot){
 const int style=ShotStyle(shot);if(!style||shot->mDead||shot->mBoard->mPaused)return;
 if(shot->mPosY+shot->mPosZ<-40||shot->mPosY+shot->mPosZ>640){shot->Die();return;}
 if(style<9)shot->mVelY=MemeShooterRules::WobbleStep(style,shot->mProjectileAge);
}
}
