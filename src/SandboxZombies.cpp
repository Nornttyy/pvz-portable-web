// Abstract newspaper replacement uses native phase/health/save fields and rig.
#include "SandboxZombies.h"
#include "SandboxArt.h"
#include "LawnApp.h"
#include "Lawn/Zombie.h"
#include "PvzpLib/Reanimator.h"
extern bool gSandboxEnabled;
namespace SandboxZombies {
bool IsPhone(const Zombie* z){return z&&int(z->mZombieType)==5&&(gSandboxEnabled||(gLawnApp&&gLawnApp->IsAdventureMode()));}
void RecoverPhone(Zombie* z){
 if(!IsPhone(z)||z->mDead||z->IsDeadOrDying()||!z->mHasHead||!z->mHasArm||z->mZombiePhase!=PHASE_NEWSPAPER_MAD||z->mPhaseCounter>0)return;
 z->StopEating();z->mZombiePhase=PHASE_NEWSPAPER_READING;z->mShieldType=SHIELDTYPE_NEWSPAPER;z->mShieldMaxHealth=150;z->mShieldHealth=100;
 if(auto* body=gLawnApp->ReanimationTryToGet(z->mBodyReanimID))body->SetImageOverride("anim_head1",nullptr);
 z->AttachShield();z->PickRandomSpeed();z->StartWalkAnim(15);RefreshDamageArt(z);
}
void Reset(){} void Forget(Zombie*){} void Assign(Zombie*,int){}
void Tick(Board*){} void DrawPortrait(Sexy::Graphics*,int,int,int,int,int){}
float Speed(const Zombie*){return 1.0f;} int Damage(const Zombie*,int damage){return damage;}
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
