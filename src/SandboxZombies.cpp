// Abstract newspaper replacement uses native phase/health/save fields and rig.
#include "SandboxZombies.h"
#include "SandboxArt.h"
#include "LawnApp.h"
#include "Lawn/Zombie.h"
#include "Lawn/Board.h"
#include "PvzpLib/Reanimator.h"
#include <cmath>
extern bool gSandboxEnabled;
namespace SandboxZombies {
namespace {
bool Enabled(){return gSandboxEnabled||(gLawnApp&&gLawnApp->IsAdventureMode());}
bool Walker(Zombie* z){return z&&z->IsOnBoard()&&!z->mDead&&!z->IsDeadOrDying()&&z->mHasHead&&!z->mMindControlled&&z->mZombiePhase==PHASE_ZOMBIE_NORMAL&&z->mZombieHeight==HEIGHT_ZOMBIE_NORMAL&&!z->mInPool;}
}
bool IsRetreating(Zombie*){return false;}
// The normal cone's native phase counter is serialized with the zombie. Only
// a real armor break starts this one-time gag; hits never push or restart it.
bool IsFeigning(Zombie* z){return Enabled()&&Walker(z)&&int(z->mZombieType)==2&&z->mHelmHealth==0&&z->mPhaseCounter>0&&z->mPhaseCounter<=300;}
void ArmorBroken(Zombie* z){
 if(!Enabled()||!Walker(z)||int(z->mZombieType)!=2||z->mHelmHealth!=0)return;
 z->mPhaseCounter=300;z->StopEating();
}
void AdjustPose(Zombie* z,Reanimation* body){
 if(!body||!IsFeigning(z))return;
 const float t=z->mPhaseCounter>275?(300-z->mPhaseCounter)/25.0f:z->mPhaseCounter<40?z->mPhaseCounter/40.0f:1.0f;
 const float angle=-1.36f*t*t*(3-2*t),c=std::cos(angle),s=std::sin(angle);
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
void Tick(Board*){} void DrawPortrait(Sexy::Graphics*,int,int,int,int,int){}
float Speed(Zombie* z){
 if(!Enabled()||!Walker(z))return 1.0f;
 if(IsFeigning(z))return 0.0f;
 const int type=int(z->mZombieType);
 if(type==0||type==2||type==4||type==6)for(auto* leader:z->mBoard->mZombies)
  if(leader!=z&&int(leader->mZombieType)==1&&Walker(leader)&&std::abs(leader->mRow-z->mRow)<=1&&std::abs(leader->mPosX-z->mPosX)<160)return 1.5f;
 return 1.0f; // Flags do not stack, and vehicles/giants keep their native pace.
}
int Damage(Zombie*,int damage){return damage;}
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
