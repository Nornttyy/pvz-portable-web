// Abstract newspaper replacement uses native phase/health/save fields and rig.
#include "SandboxZombies.h"
#include "SandboxArt.h"
#include "LawnApp.h"
#include "Lawn/Zombie.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "PvzpLib/Reanimator.h"
#include <algorithm>
#include <cmath>
#include <vector>
extern bool gSandboxEnabled;
namespace SandboxZombies {
namespace {
bool Enabled(){return gSandboxEnabled||(gLawnApp&&gLawnApp->IsAdventureMode());}
bool Walker(Zombie* z){return z&&z->IsOnBoard()&&!z->mDead&&!z->IsDeadOrDying()&&z->mHasHead&&!z->mMindControlled&&z->mZombiePhase==PHASE_ZOMBIE_NORMAL&&z->mZombieHeight==HEIGHT_ZOMBIE_NORMAL&&!z->mInPool;}
}
bool IsRetreating(Zombie* z){return Enabled()&&Walker(z)&&int(z->mZombieType)==24&&z->mZombieAge%600>=400&&z->mZombieAge%600<500&&z->mPosX<760;}
// The normal cone's native phase counter is serialized with the zombie. Only
// a real armor break starts this one-time gag; hits never push or restart it.
bool IsFeigning(Zombie* z){return Enabled()&&Walker(z)&&int(z->mZombieType)==2&&z->mHelmHealth==0&&z->mPhaseCounter>0&&z->mPhaseCounter<=300;}
bool IsResting(Zombie* z){
 if(!Enabled()||!Walker(z))return false;
 const int age=z->mZombieAge%800,type=int(z->mZombieType);
 return (type==0&&age>=600)||(type==7&&age>=200&&age<350);
}
void ArmorBroken(Zombie* z){
 if(!Enabled()||!Walker(z)||int(z->mZombieType)!=2||z->mHelmHealth!=0)return;
 z->mPhaseCounter=300;z->StopEating();
}
void AdjustPose(Zombie* z,Reanimation* body){
 if(!body||!Enabled())return;
 float angle=0;
 if(IsRetreating(z)){
  angle=0.24f*std::sin((z->mZombieAge%600-400)*3.14159265f/100);
 }else if(int(z->mZombieType)==21&&z->mZombiePhase==PHASE_LADDER_CARRYING&&z->mPhaseCounter>0&&z->mPhaseCounter<=35){
  angle=0.18f*std::sin(z->mPhaseCounter*3.14159265f/35);
 }else if(IsFeigning(z)){
  const float t=z->mPhaseCounter>275?(300-z->mPhaseCounter)/25.0f:z->mPhaseCounter<40?z->mPhaseCounter/40.0f:1.0f;
  angle=-1.36f*t*t*(3-2*t);
 }else if(IsResting(z)){
  const int start=int(z->mZombieType)==0?600:200,end=int(z->mZombieType)==0?800:350,age=z->mZombieAge%800;
  const float fade=std::clamp(std::min(age-start,end-age)/20.0f,0.0f,1.0f);
  angle=(int(z->mZombieType)==0?0.18f:-0.14f)*fade+0.015f*fade*std::sin(age*0.10f);
 }else if(Walker(z)&&int(z->mZombieType)==7&&z->mZombieAge%800<200&&!z->mIsEating){
  const int age=z->mZombieAge%800;angle=-0.12f*std::clamp(std::min(age,200-age)/20.0f,0.0f,1.0f);
 }else if(Walker(z)&&int(z->mZombieType)==6&&z->mPhaseCounter>0&&z->mPhaseCounter<=20)angle=-0.09f*std::sin(z->mPhaseCounter*3.14159265f/20);
 else return;
 const float c=std::cos(angle),s=std::sin(angle);
 auto& m=body->mOverlayMatrix;
 // Rotate the complete native rig around its feet, including arm/head parts.
 const float footX=m.m00*45+m.m01*120+m.m02,footY=m.m10*45+m.m11*120+m.m12;
 const float a=m.m00,b=m.m01,d=m.m10,e=m.m11;
 m.m00=c*a-s*d;m.m01=c*b-s*e;m.m10=s*a+c*d;m.m11=s*b+c*e;
 m.m02=footX-m.m00*45-m.m01*120;m.m12=footY-m.m10*45-m.m11*120;
}
bool IsPhone(const Zombie* z){return z&&int(z->mZombieType)==5&&(gSandboxEnabled||(gLawnApp&&gLawnApp->IsAdventureMode()));}
void RecoverPhone(Zombie* z){
 if(!IsPhone(z)||z->mDead||z->IsDeadOrDying()||!z->mHasHead||!z->mHasArm||z->mZombiePhase!=PHASE_NEWSPAPER_MAD||z->mPhaseCounter>0)return;
 z->StopEating();z->mZombiePhase=PHASE_NEWSPAPER_READING;z->mShieldType=SHIELDTYPE_NEWSPAPER;z->mShieldMaxHealth=150;z->mShieldHealth=100;
 if(auto* body=gLawnApp->ReanimationTryToGet(z->mBodyReanimID))body->SetImageOverride("anim_head1",nullptr);
 z->AttachShield();z->PickRandomSpeed();z->StartWalkAnim(15);RefreshDamageArt(z);
}
void Reset(){} void Forget(Zombie*){} void Assign(Zombie*,int){}
void Tick(Board* b){
 if(!Enabled()||b->mPaused)return;
 struct Delivery{float x;int row,wave;};std::vector<Delivery> pending;
 for(auto* z:b->mZombies){
  if(IsResting(z))z->StopEating();
  if(IsRetreating(z)){
   z->StopEating();if(z->mZombieAge%600==400&&!z->mIceTrapCounter&&!z->mButteredCounter)gLawnApp->PlayMemeCue(3,6);
  }
  // Use saved native age/phase fields. A ladder changes lanes only before
  // placing it, only on land, and only toward a genuinely less crowded lane.
  if(!z->mDead&&z->IsOnBoard()&&!z->IsDeadOrDying()&&z->mHasHead&&!z->mMindControlled&&int(z->mZombieType)==21&&z->mShieldHealth>0&&z->mZombiePhase==PHASE_LADDER_CARRYING&&z->mZombieHeight==HEIGHT_ZOMBIE_NORMAL&&!z->mInPool&&!z->mIsEating&&!z->mIceTrapCounter&&!z->mButteredCounter&&z->mZombieAge>0&&z->mZombieAge%600==0&&z->mPosX>100&&z->mPosX<800){
   auto crowd=[&](int row){int n=0;for(auto* p:b->mPlants)if(!p->mDead&&!p->mSquished&&p->mRow==row&&int(p->mSeedType)!=16&&int(p->mSeedType)!=33&&int(p->mSeedType)!=21&&int(p->mSeedType)!=46&&p->mX<z->mPosX+60&&p->mX>z->mPosX-180)++n;return n;};
   int best=z->mRow,count=crowd(best);
   for(int row:{z->mRow-1,z->mRow+1})if(row>=0&&row<(b->StageHasPool()?6:5)&&b->RowCanHaveZombies(row)&&!(b->StageHasPool()&&(row==2||row==3||z->mRow==2||z->mRow==3))){const int n=crowd(row);if(n<count){count=n;best=row;}}
   if(best!=z->mRow){z->StopEating();z->SetRow(best);z->mPhaseCounter=35;gLawnApp->PlayMemeCue(2,-5);}
  }
  // Age is a native saved field: a single delivery, not a timer reset on load.
  if(!z->mDead&&z->IsOnBoard()&&!z->IsDeadOrDying()&&z->mHasHead&&!z->mMindControlled&&int(z->mZombieType)==16&&z->IsFlying()&&z->mZombieAge==600&&z->mPosX>100&&z->mPosX<740&&!(b->StageHasPool()&&(z->mRow==2||z->mRow==3)))pending.push_back({z->mPosX+25,z->mRow,z->mFromWave});
 }
 for(const auto& drop:pending)if(b->mZombies.mSize<b->mZombies.mMaxSize-8){
  if(auto* z=b->AddZombieInRow(ZOMBIE_NORMAL,drop.row,drop.wave)){
   z->mPosX=drop.x;z->mX=int(drop.x);z->mAltitude=80;z->mZombieHeight=HEIGHT_FALLING;z->UpdateReanim();
  }
 }
}
void DrawPortrait(Sexy::Graphics*,int,int,int,int,int){}
float Speed(Zombie* z){
 if(!Enabled()||!Walker(z))return 1.0f;
 if(IsRetreating(z))return -1.4f;
 if(IsFeigning(z)||IsResting(z))return 0.0f;
 const int type=int(z->mZombieType);
 if(type==7&&z->mZombieAge%800<200)return 1.8f;
 if(type==0||type==2||type==4||type==6)for(auto* leader:z->mBoard->mZombies)
  if(leader!=z&&int(leader->mZombieType)==1&&Walker(leader)&&std::abs(leader->mRow-z->mRow)<=1&&std::abs(leader->mPosX-z->mPosX)<160)return 1.5f;
 return 1.0f; // Flags do not stack, and vehicles/giants keep their native pace.
}
int Damage(Zombie* z,int damage,unsigned flags){
 static bool sharing=false;
 if(sharing||!Enabled()||!Walker(z)||int(z->mZombieType)==6||damage<2||damage>100||flags)return damage;
 Zombie* guard=nullptr;float nearest=10000;
 for(auto* other:z->mBoard->mZombies)if(other!=z&&Walker(other)&&int(other->mZombieType)==6&&other->mShieldHealth>0&&std::abs(other->mRow-z->mRow)<=1&&std::abs(other->mPosX-z->mPosX)<100){
  const float distance=std::abs(other->mPosX-z->mPosX)+100*std::abs(other->mRow-z->mRow);if(distance<nearest){guard=other;nearest=distance;}
 }
 if(!guard)return damage;
 const int share=std::min(guard->mShieldHealth,damage/2);sharing=true;guard->TakeDamage(share,0);sharing=false;guard->mPhaseCounter=20;
 return damage-share;
}
bool ElectricHit(Zombie*){return false;} void CombatDeath(Zombie*){}
void DrawEffects(Sexy::Graphics*,Board*,int){}
bool HasShot(const Projectile*){return false;} bool DrawShot(Sexy::Graphics*,const Projectile*){return false;}
bool Impact(Projectile*,Plant*){return false;} Plant* CollisionTarget(Projectile*){return nullptr;}
void ForgetShot(Projectile*){} void ForgetPlant(Plant*){} bool AttackSlowed(const Plant*){return false;}
void RefreshDamageArt(Zombie* z){
 if(!IsPhone(z))return;auto* body=gLawnApp->ReanimationTryToGet(z->mBodyReanimID);if(!body)return;
 if(body->TrackExists("Zombie_paper_paper"))body->SetImageOverride("Zombie_paper_paper",SandboxArt::Phone(z->mShieldHealth<50?2:z->mShieldHealth<100?1:0));
 for(const auto* name:{"Zombie_paper_hands","Zombie_paper_hands2"})if(body->TrackExists(name))body->SetImageOverride(name,z->mShieldHealth>0?SandboxArt::PhoneHands(name[18]=='2'?"Zombie_paper_hands2.png":"Zombie_paper_hands.png"):nullptr);
}
Sexy::Image* DetachedArmor(const Zombie* z){return IsPhone(z)?SandboxArt::Phone(2):nullptr;}
}
