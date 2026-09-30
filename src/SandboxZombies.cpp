// Louis uses the native headless rig, not the engine's terminal head-loss state.
#include "SandboxZombies.h"
#include "LawnApp.h"
#include "SandboxArt.h"
#include "Lawn/Board.h"
#include "Lawn/Zombie.h"
#include "Lawn/System/ReanimationLawn.h"
#include "PvzpLib/Reanimator.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"
#include <map>
#include <memory>
namespace SandboxZombies {
namespace {
std::map<const Zombie*,int> identities;
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
int Type(const Zombie* z){if(!z)return -1;const auto it=identities.find(z);return it==identities.end()?int(z->mZombieType):it->second;}
bool IsLouis(const Zombie* z){return z&&Type(z)==Louis;}
void Reset(){identities.clear();}
void Forget(Zombie* z){identities.erase(z);}
bool Restore(Zombie* z,int id){
 const auto* d=Find(id);if(!z||z->mDead||!d||int(z->mZombieType)!=d->base)return false;
 identities[z]=id;
 // mHasHead is also the native alive/targetable/eating gate. Do not set it
 // false at spawn, heal it on load, or change a wounded zombie's saved state.
 z->SetupReanimForLostHead();return true;
}
void Assign(Zombie* z,int id){Restore(z,id);}
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
void DrawPortrait(Sexy::Graphics* g,int x,int y,int w,int h,int id){
 if(id!=Louis)return;
 // Separate cache: the native zombie portrait must keep its head.
 static std::unique_ptr<Sexy::MemoryImage> portrait;
 if(!portrait){
  portrait=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(200,210);Sexy::Graphics canvas(portrait.get());canvas.SetLinearBlend(true);
  Reanimation anim;anim.ReanimationInitializeType(40,40,REANIM_ZOMBIE);anim.SetFramesForLayer("anim_idle");Zombie::SetupReanimLayers(&anim,ZOMBIE_NORMAL);
  for(const char* prefix:{"anim_head","anim_hair","anim_tongue"})anim.AssignRenderGroupToPrefix(prefix,RENDER_GROUP_HIDDEN);
  anim.Draw(&canvas);
 }
 SandboxArt::DrawFit(g,portrait.get(),x+4,y+5,w-8,h-10);
}
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
void RefreshDamageArt(Zombie* z){if(IsLouis(z))z->SetupReanimForLostHead();}
Sexy::Image* DetachedArmor(const Zombie*){return nullptr;}
}
