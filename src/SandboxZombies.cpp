// Louis uses the native headless rig, not the engine's terminal head-loss state.
#include "SandboxZombies.h"
#include "LawnApp.h"
#include "SandboxArt.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
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
bool IsRunner(const Zombie* z){return z&&Type(z)==Runner;}
bool IsRunning(const Zombie* z){return IsRunner(z)&&!z->mDead&&z->mHasHead&&int(z->mZombiePhase)>=RunIn&&int(z->mZombiePhase)<=RunOut;}
void Reset(){identities.clear();}
void Forget(Zombie* z){identities.erase(z);}
bool Restore(Zombie* z,int id){
 const auto* d=Find(id);if(!z||z->mDead||!d||int(z->mZombieType)!=d->base)return false;
 identities[z]=id;
 // mHasHead is also the native alive/targetable/eating gate. Do not set it
 // false at spawn, heal it on load, or change a wounded zombie's saved state.
 if(id==Louis)z->SetupReanimForLostHead();return true;
}
void Assign(Zombie* z,int id){
 if(!Restore(z,id)||id!=Runner)return;
 // These native fields are already serialized. Restore() deliberately does
 // not reset them: a saved fleeing runner must never charge in a second time.
 z->mZombiePhase=static_cast<decltype(z->mZombiePhase)>(RunIn);
 z->mPhaseCounter=0;z->mTargetCol=-1;z->mHasObject=false;z->StopEating();
 z->PlayZombieReanim("anim_walk2",REANIM_LOOP,0,34.0f);
}
bool UpdateRunner(Zombie* z){
 if(!IsRunner(z)||!z->mBoard||!z->IsOnBoard()||z->IsDeadOrDying())return false;
 if(!z->mHasHead){
  if(int(z->mZombiePhase)>=RunIn&&int(z->mZombiePhase)<=RunOut){z->mZombiePhase=PHASE_ZOMBIE_NORMAL;z->StartWalkAnim(10);}
  return false; // Native injury/death, never an invulnerable getaway.
 }
 if(!IsRunning(z))return false;
 z->StopEating();
 if(z->IsImmobilizied())return true;
 if(z->mTargetCol<0){
  int last=9;for(auto* p:z->mBoard->mPlants)if(!p->mDead&&!p->mSquished&&!p->NotOnGround()&&p->mRow==z->mRow)last=std::min(last,p->mPlantCol);
  z->mTargetCol=last==9?0:last;
 }
 if(z->mMindControlled){z->mZombiePhase=static_cast<decltype(z->mZombiePhase)>(RunOut);z->mHasObject=true;}
 if(int(z->mZombiePhase)==RunBrake){
  if(z->mPhaseCounter>0)--z->mPhaseCounter;
  if(z->mPhaseCounter==0){z->mZombiePhase=static_cast<decltype(z->mZombiePhase)>(RunOut);z->mHasObject=true;z->UpdateAnimSpeed();}
  return true;
 }
 const float speed=(int(z->mZombiePhase)==RunOut?RunOutSpeed:RunInSpeed)*(z->IsMovingAtChilledSpeed()?0.5f:1.0f);
 if(int(z->mZombiePhase)==RunIn){
  const float turnX=std::min(z->mPosX,float(std::max(40,z->mBoard->GridToPixelX(std::clamp(z->mTargetCol,0,8),z->mRow)-25)));
  z->mPosX=std::max(turnX,z->mPosX-speed);
  if(z->mPosX<=turnX){z->mZombiePhase=static_cast<decltype(z->mZombiePhase)>(RunBrake);z->mPhaseCounter=BrakeTicks;z->UpdateAnimSpeed();}
 }else z->mPosX+=speed;
 z->mX=int(z->mPosX);
 // Native CheckForBoardEdge handles the right-side exit and level award.
 return true;
}
void Tick(Board*){}
bool HasInteraction(const Zombie* z){return LegacyPhase(z)||IsRunning(z);}
bool IsHeld(const Zombie*){return false;}
bool UpdateInteraction(Zombie* z){ReleaseLegacy(z);return false;}
bool CatchForReturn(Plant*,Zombie*){return false;}
bool Staple(Zombie*){return false;}
bool Slip(Zombie*){return false;}
bool Misdirect(Zombie*){return false;}
bool IsRetreating(Zombie* z){return IsRunner(z)&&z->mHasObject;}
bool IsFeigning(Zombie*){return false;}
bool IsResting(Zombie*){return false;}
void PoleLanded(Zombie*){}
void ArmorBroken(Zombie*){}
void AdjustPose(Zombie* z,Reanimation* body){
 if(!body||!IsRunning(z))return;
 // Lean around the feet; native mirroring remains responsible for facing.
 const float lean=(int(z->mZombiePhase)==RunBrake?-0.10f:0.13f)*(IsRetreating(z)?-1.0f:1.0f)*z->mScaleZombie;
 body->mOverlayMatrix.m01+=lean;body->mOverlayMatrix.m02-=lean*120.0f;
}
bool IsPhone(const Zombie*){return false;}
void RecoverPhone(Zombie*){}
void DrawPortrait(Sexy::Graphics* g,int x,int y,int w,int h,int id){
 if(!Find(id))return;
 // Separate cache: the native zombie portrait must keep its head.
 static std::array<std::unique_ptr<Sexy::MemoryImage>,2> portraits;
 auto& portrait=portraits[id==Runner?1:0];
 if(!portrait){
  portrait=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(200,210);Sexy::Graphics canvas(portrait.get());canvas.SetLinearBlend(true);
  Reanimation anim;anim.ReanimationInitializeType(40,40,REANIM_ZOMBIE);anim.SetFramesForLayer(id==Runner?"anim_walk2":"anim_idle");Zombie::SetupReanimLayers(&anim,ZOMBIE_NORMAL);
  if(id==Louis)for(const char* prefix:{"anim_head","anim_hair","anim_tongue"})anim.AssignRenderGroupToPrefix(prefix,RENDER_GROUP_HIDDEN);
  if(id==Runner){anim.mAnimTime=0.35f;anim.mOverlayMatrix.m01=0.13f;anim.mOverlayMatrix.m02-=15.6f;}
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
