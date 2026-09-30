// All zombies use their native mechanics and artwork. Keep no-op hooks for ABI stability.
#include "SandboxZombies.h"
#include "Lawn/Board.h"
#include "Lawn/Zombie.h"
#include "PvzpLib/Reanimator.h"
namespace SandboxZombies {
namespace {
bool LegacyPhase(const Zombie* z){return z&&int(z->mZombiePhase)>=Held&&int(z->mZombiePhase)<=AirDrop;}
void ReleaseLegacy(Zombie* z){
 if(!LegacyPhase(z))return;
 z->StopEating();
 z->mZombiePhase=int(z->mZombieType)==21&&z->mShieldHealth>0?PHASE_LADDER_CARRYING:PHASE_ZOMBIE_NORMAL;
 z->mPhaseCounter=0;z->mTargetPlantID=static_cast<decltype(z->mTargetPlantID)>(0);z->mTargetRow=-1;
 z->mAltitude=0;z->mZombieHeight=HEIGHT_ZOMBIE_NORMAL;
 z->mPosY=z->GetPosYBasedOnRow(z->mRow);z->mX=int(z->mPosX);z->mY=int(z->mPosY);
 if(!z->mDead&&!z->IsDeadOrDying())z->StartWalkAnim(10);
}
}
void RestoreNative(Board* b){
 // Release saved holds, pins, hops and carries without recreating enemies,
 // healing armor, changing identities or resetting player progression.
 for(auto* z:b->mZombies)ReleaseLegacy(z);
}
void Reset(){} void Forget(Zombie*){} void Assign(Zombie*,int){}
void Tick(Board*){}
bool HasInteraction(const Zombie* z){return LegacyPhase(z);}
bool IsHeld(const Zombie*){return false;}
bool UpdateInteraction(Zombie* z){ReleaseLegacy(z);return false;}
bool CatchForReturn(Plant*,Zombie*){return false;}
bool Staple(Zombie*){return false;}
bool Slip(Zombie*){return false;}
bool Misdirect(Zombie*){return false;}
bool IsRetreating(Zombie*){return false;}
bool IsFeigning(Zombie*){return false;}
bool IsResting(Zombie*){return false;}
void PoleLanded(Zombie*){}
void ArmorBroken(Zombie*){}
void AdjustPose(Zombie*,Reanimation*){}
bool IsPhone(const Zombie*){return false;}
void RecoverPhone(Zombie*){}
void DrawPortrait(Sexy::Graphics*,int,int,int,int,int){}
float Speed(Zombie*){return 1.0f;}
int Damage(Zombie*,int damage,unsigned){return damage;}
bool ElectricHit(Zombie*){return false;}
void CombatDeath(Zombie*){}
void DrawEffects(Sexy::Graphics*,Board*,int){}
bool HasShot(const Projectile*){return false;}
bool DrawShot(Sexy::Graphics*,const Projectile*){return false;}
bool Impact(Projectile*,Plant*){return false;}
Plant* CollisionTarget(Projectile*){return nullptr;}
void ForgetShot(Projectile*){}
void ForgetPlant(Plant*){}
bool AttackSlowed(const Plant*){return false;}
void RefreshDamageArt(Zombie*){}
Sexy::Image* DetachedArmor(const Zombie*){return nullptr;}
}
