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
