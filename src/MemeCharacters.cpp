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
std::map<const Projectile*,int> shotStyles; // 1..8 legacy wobble; 9 burst; 10 hit; 11..18 legacy miss; 19 retreat ice; 20 legacy straight miss; 32..287 floating seeds.
bool NativeSequence(int id){return id>=508&&id<=513;}
void Burst(State& s){
 s.phase=1;s.heat=0;s.remaining=MemeShooterRules::BurstCount;s.delay=0;s.timer=0;s.pulse=30;
 gLawnApp->PlayRageRelease(); // Once per release, never once per pea or save restore.
}
// Native zombie IDs are stable through DataArray recycling; never retain pointers.
std::map<unsigned,int> laneCooldown;
bool Enemy(Zombie* z){return !z->mDead&&z->IsOnBoard()&&!z->mMindControlled&&!z->IsDeadOrDying()&&z->mHasHead;}
int VisualY(const Plant* p){return p->mY+int(std::lround(PlantDrawHeightOffset(p->mBoard,const_cast<Plant*>(p),p->mSeedType,p->mPlantCol,p->mRow)));}
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
 if(id==507)states[p].remaining=3;
 if(!NativeSequence(id)){p->mLaunchCounter=id==503?std::clamp(p->mLaunchCounter,300,2500):9999;p->mShootingCounter=0;}
 if(id==502)p->SetSleeping(false); // Keep the native short-range shot and add a daytime lure.
}
std::array<int,10> Save(const Plant* p){auto it=states.find(p);if(it==states.end())return {};const auto& s=it->second;return {s.id,s.health,s.phase,s.heat,s.timer,s.delay,s.age,s.pulse,s.remaining,s.direction};}
bool Restore(Plant* p,const std::array<int,10>& a){
 const bool shooter=a[0]==500,newShooter=shooter&&a[9]==2;
 const auto* d=Find(a[0]);if(!d||int(p->mSeedType)!=d->base||p->mDead||a[1]<0||a[1]>p->mPlantMaxHealth||a[2]<0||a[2]>(newShooter?1:2)||a[3]<0||a[3]>(newShooter?300:1000)||a[4]<0||a[4]>2000||a[5]<0||a[5]>2000||a[6]<0||a[6]>=1000000||a[7]<0||a[7]>50||a[8]<0||a[8]>(newShooter?150:(a[0]==506||NativeSequence(a[0]))?6:3)||(!newShooter&&a[9]!=1&&a[9]!=-1))return false;
 if((a[0]==508||a[0]==512)&&a[8]>5)return false;
 if(a[0]==511&&(a[3]>7||a[8]>4))return false;
 if(a[0]==512&&a[3]>2)return false;
 if(newShooter&&((a[2]==1&&(a[8]==0||a[3]!=0))||(a[2]==0&&a[8]!=0)))return false;
 if(a[0]==507&&a[8]<1)return false;
 State s{a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9]};
 // Direction was unused for this character: 2 versions the new burst state.
 // Migrate old overheating saves without healing or inventing a free volley.
 if(shooter&&!newShooter){s.phase=0;s.heat=a[2]==0?a[3]*300/1000:0;s.timer=0;s.delay=std::min(a[5],150);s.remaining=0;s.pulse=0;s.direction=2;}
 if(shooter)s.heat=std::min(s.heat,MemeShooterRules::MaxRage); // Keep old 300-rage saves readable.
 if(shooter)s.remaining=std::min(s.remaining,MemeShooterRules::BurstCount); // Read old 150-pea saves without adding shots.
 states[p]=s;if(!NativeSequence(a[0])){p->mLaunchCounter=a[0]==503?std::clamp(p->mLaunchCounter,0,2500):9999;p->mShootingCounter=0;}return true;
}
int Data(const Plant* p,int field){const auto it=states.find(p);if(it==states.end())return -1;const auto& s=it->second;return field==0?s.phase:field==1?s.heat:field==2?s.timer:field==3?s.direction:s.remaining;}
// Keep legacy input/ABI callers harmless; rage is automatic only.
bool Activate(Plant*,int){return false;}
bool Click(Board*,int,int){return false;}
bool RearmPotato(Plant* p){
 auto it=states.find(p);if(it==states.end()||it->second.id!=507||p->mDead||p->mPlantHealth<=0||it->second.remaining<=1)return false;
 --it->second.remaining;it->second.timer=600;it->second.pulse=40;return true;
}
Zombie* PickTarget(Plant* p,Zombie* nativeTarget){
 if(Type(p)!=510||!nativeTarget)return nativeTarget;
 auto* last=nativeTarget;
 for(auto* z:p->mBoard->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mPosX>last->mPosX&&z->mPosX<800&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY)))last=z;
 return last;
}
bool ButterReady(const Plant* p){auto it=states.find(p);return it!=states.end()&&it->second.id==512&&it->second.heat==2;}
bool StarTarget(Plant* p){
 for(auto* z:p->mBoard->mZombies)if(Enemy(z)&&std::abs(z->mPosX-p->mX)<450&&std::abs(z->mRow-p->mRow)<=2&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY)))return true;
 return false;
}
void Tick(Board* b){
 if(b->mPaused)return;
 for(auto it=laneCooldown.begin();it!=laneCooldown.end();)if(--it->second<=0||!b->ZombieTryToGet(static_cast<ZombieID>(it->first)))it=laneCooldown.erase(it);else ++it;
 for(auto* p:b->mPlants){
  auto it=states.find(p);if(it==states.end())continue;if(p->mDead){states.erase(it);continue;}auto& s=it->second;
  if(!NativeSequence(s.id)){if(s.id!=503)p->mLaunchCounter=9999;p->mShootingCounter=0;}
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
    if(target){s.timer=300;s.pulse=50;s.phase=1;s.heat=int(target->mPosX-p->mX+65);gLawnApp->PlayFoley(FOLEY_THROW);}
   }
   if(s.phase==1&&s.pulse==28){
    for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mPosX>=p->mX-65&&z->mPosX<p->mX+40&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))){
     z->TakeDamage(80,0);break;
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
     auto* shot=b->AddProjectile(p->mX+12,VisualY(p)+22,p->mRenderOrder-1,p->mRow,PROJECTILE_PEA);
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
  }else if(s.id==505){
   if(!s.delay&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8)if(auto* target=p->FindTargetZombie(p->mRow,WEAPON_PRIMARY)){
    Shoot(p,target);s.delay=300;s.pulse=20;
   }
  }else if(s.id==506){
   if(s.remaining&&!s.delay&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8){
    Shoot(p,nullptr);--s.remaining;s.delay=30;s.pulse=20;
   }
  }else if(s.id==508&&s.remaining){
   p->mLaunchCounter=std::max(p->mLaunchCounter,300);
   if(p->mState!=STATE_CACTUS_LOW&&p->mState!=STATE_CACTUS_HIGH){s.remaining=0;continue;}
   if(!s.delay&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8){
    const auto weapon=p->mState==STATE_CACTUS_HIGH?WEAPON_PRIMARY:WEAPON_SECONDARY;
    p->Fire(nullptr,p->mRow,weapon);--s.remaining;s.delay=8;s.pulse=20;
   }
  }else if(s.id==512&&s.remaining){
   p->mLaunchCounter=std::max(p->mLaunchCounter,200);
   if(!s.timer){s.remaining=0;continue;}
   if(!s.delay&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8){
    Zombie* target=nullptr;
    for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mButteredCounter>0&&z->mPosX>=p->mX-20&&z->mPosX<800&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))&&(!target||z->mPosX<target->mPosX))target=z;
    if(target){p->Fire(target,p->mRow,WEAPON_PRIMARY);--s.remaining;s.delay=12;s.pulse=20;}else s.delay=10;
   }
  }else if(s.id==513){
   if(p->mState!=STATE_READY){
    s.phase=1;if(++s.heat>=100){s.remaining=std::min(6,s.remaining+1);s.heat=0;}
   }else{
    s.phase=0;s.heat=0;
    if(s.remaining){
     p->mLaunchCounter=std::max(p->mLaunchCounter,150);
     if(!s.delay&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8)if(auto* target=p->FindTargetZombie(p->mRow,WEAPON_PRIMARY)){
      p->Fire(target,p->mRow,WEAPON_PRIMARY);--s.remaining;s.delay=12;s.pulse=20;
     }
    }
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
 if(s.id==507)horizontal=vertical=0.6f+0.2f*(s.remaining-1);
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
 if((s.id==505||s.id==506)&&s.pulse){const float t=s.pulse/20.0f;horizontal-=0.07f*std::sin(t*3.14159265f);vertical+=0.05f*std::sin(t*3.14159265f);}
 if(NativeSequence(s.id)&&s.pulse){const float wave=std::sin(s.pulse*3.14159265f/20);horizontal+=0.065f*wave;vertical-=0.06f*wave;}
 if(s.id==511&&s.pulse){x+=sx*3*std::sin(s.pulse*0.3f);y-=sy*4*std::sin(s.pulse*3.14159265f/20);}
 if(s.id==503&&(s.phase==1||s.phase==2)){
  const float crouch=s.phase==1?1-std::clamp(s.pulse/20.0f,0.0f,1.0f):std::clamp(s.pulse/30.0f,0.0f,1.0f);
  vertical=1-0.72f*crouch;horizontal=1+0.15f*crouch;
 }
 if(s.id==504&&s.phase==1){const float t=(80-s.timer)/80.0f,travel=std::sin(3.14159265f*t);x+=std::abs(sx)*s.heat*travel;y-=sy*55*std::abs(std::sin(6.2831853f*t));horizontal=1+0.12f*travel;vertical=1-0.12f*travel;}
 x+=40*sx*(1-horizontal);y+=65*sy*(1-vertical);sx*=horizontal;sy*=vertical;
}
void Effects(Sexy::Graphics* g,Board* b,int row){
 for(const auto& [p,s]:states)if(!p->mDead&&p->mRow==row&&!p->mSquished&&!const_cast<Plant*>(p)->NotOnGround()){
  const int x=p->mX,y=VisualY(p);
  if(s.id==500){
   g->SetColor(Sexy::Color(57,37,18));g->FillRect(x+12,y+77,56,6);
   g->SetColor(s.phase==1?Sexy::Color(210,72,43):Sexy::Color(226,167,64));g->FillRect(x+13,y+78,s.phase==1?s.remaining*54/MemeShooterRules::BurstCount:s.heat*54/MemeShooterRules::MaxRage,4);
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
void OnFired(Plant* p,Projectile* shot){
 // The native attack animation owns the first shot; extras keep its native
 // projectile, muzzle, hit rules and saved velocity. They never recursively
 // refill their own queue.
 const int id=Type(p);
 for(auto* other:p->mBoard->mPlants)if(Type(other)==509&&other!=p&&!other->mDead&&!other->mIsAsleep&&!other->mSquished&&!other->NotOnGround()&&other->mPlantHealth>0&&other->mState==STATE_CHOMPER_DIGESTING&&std::abs(other->mPlantCol-p->mPlantCol)+std::abs(other->mRow-p->mRow)==1){
  auto& s=states.at(other);if(!s.timer){other->mStateCountdown=std::max(0,other->mStateCountdown-200);s.timer=50;s.pulse=20;}
 }
 if(id==508){auto& s=states.at(p);if(!s.remaining){s.remaining=5;s.delay=8;p->mLaunchCounter=340;}s.pulse=20;}
 if(id==510)states.at(p).pulse=20;
 if(id==511){auto& s=states.at(p);const float angle=s.heat*3.14159265f/8,c=std::cos(angle),v=std::sin(angle),x=shot->mVelX,y=shot->mVelY;shot->mVelX=x*c-y*v;shot->mVelY=x*v+y*c;s.pulse=20;if(++s.remaining==5){s.remaining=0;s.heat=(s.heat+1)%8;}}
 if(id==512){auto& s=states.at(p);if(!s.remaining){if(shot->mProjectileType==PROJECTILE_BUTTER){s.heat=0;s.remaining=5;s.delay=130;s.timer=400;}else s.heat=std::min(2,s.heat+1);}s.pulse=20;}
 // Echoes observe native firing events too, but never echo another echo.
 if(Type(p)!=506)for(auto* other:p->mBoard->mPlants)if(Type(other)==506&&other!=p&&!other->mDead&&!other->mIsAsleep&&!other->mSquished&&!other->NotOnGround()&&other->mPlantHealth>0&&std::abs(other->mPlantCol-p->mPlantCol)+std::abs(other->mRow-p->mRow)==1){auto& echo=states.at(other);echo.remaining=std::min(6,echo.remaining+1);echo.pulse=20;}
 if(Type(p)==505){shotStyles[shot]=19;return;}
 if(Type(p)==500){
 float x,y;if(SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(p->mHeadReanimID),"idle_mouth",35,49,32,24.5f,x,y)){shot->mPosX=p->mX+x-12;shot->mPosY=p->mY+y-12-shot->mPosZ;shot->mX=int(shot->mPosX);shot->mY=int(shot->mPosY+shot->mPosZ);}
 const auto& s=states.at(p);shot->mMotionType=MOTION_STAR;
 if(s.phase==1){
  shotStyles[shot]=9;const float angle=MemeShooterRules::SpreadAngle(Sexy::Rand(1001)),speed=MemeShooterRules::BurstSpeed(Sexy::Rand(61));
  shot->mVelX=speed*std::cos(angle);shot->mVelY=speed*std::sin(angle);
 }else{
  const bool hit=MemeShooterRules::NormalStyle(Sexy::Rand(10))==10;
  const int style=hit?10:MemeShooterRules::FloatingFirst+Sexy::Rand(256);
  shotStyles[shot]=style;shot->mVelX=3.33f;shot->mVelY=hit?0:MemeShooterRules::FloatingStep(style,0);
 }
}}
int ShotStyle(const Projectile* shot){const auto it=shotStyles.find(shot);return it==shotStyles.end()?0:it->second;}
bool CanHit(const Projectile* shot){return MemeShooterRules::CanHit(ShotStyle(shot));}
void OnImpact(Projectile* shot,Zombie* z){
 if(ShotStyle(shot)==19&&shot->mProjectileType==PROJECTILE_SNOWPEA&&z&&Walker(z)){
  z->StopEating();z->mPosX=std::min(850.0f,z->mPosX+40);z->UpdateReanim();
 }
}
bool RestoreShotStyle(const Projectile* shot,int style){
 if(style<0||(style>20&&!MemeShooterRules::IsFloating(style))||shot->mDead)return false;
 if(style&&((style==19?shot->mMotionType!=MOTION_STRAIGHT:shot->mMotionType!=MOTION_STAR)||(shot->mProjectileType!=PROJECTILE_PEA&&shot->mProjectileType!=PROJECTILE_SNOWPEA&&shot->mProjectileType!=PROJECTILE_FIREBALL)))return false;
 if(style)shotStyles[shot]=style;else shotStyles.erase(shot);return true;
}
void ForgetShot(const Projectile* shot){shotStyles.erase(shot);}
void UpdateShot(Projectile* shot){
 const int style=ShotStyle(shot);if(!style||shot->mDead||shot->mBoard->mPaused)return;
 if(shot->mPosY+shot->mPosZ<-40||shot->mPosY+shot->mPosZ>640){shot->Die();return;}
 if(MemeShooterRules::IsFloating(style))shot->mVelY=MemeShooterRules::FloatingStep(style,shot->mProjectileAge);
 else if(style<9)shot->mVelY=MemeShooterRules::WobbleStep(style,shot->mProjectileAge);
 else if(style>=11&&style<=18)shot->mVelY=MemeShooterRules::MissStep(style,shot->mProjectileAge);
}
}
