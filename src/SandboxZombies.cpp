// Louis uses the native headless rig, not the engine's terminal head-loss state.
#include "SandboxZombies.h"
#include "LawnApp.h"
#include "SandboxArt.h"
#include "SandboxPlants.h"
#include "AbstractRigVisuals.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Projectile.h"
#include "Lawn/Zombie.h"
#include "Lawn/System/ReanimationLawn.h"
#include "PvzpLib/Reanimator.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"
#include <map>
#include <memory>
#include <set>
#include <cmath>
namespace SandboxZombies {
namespace {
std::map<const Zombie*,int> identities;
std::map<const Projectile*,std::set<const Zombie*>> dodged;
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
bool IsConeWrap(const Zombie* z){return z&&Type(z)==ConeWrap;}
bool IsGiantImp(const Zombie* z){return z&&Type(z)==GiantImp;}
bool IsClever(const Zombie* z){return z&&(Type(z)==Clever||Type(z)==CleverCone);}
bool IsDodging(const Zombie* z){return IsClever(z)&&int(z->mZombiePhase)==CleverFlip&&!z->mDead&&z->mHasHead;}
bool IsRunning(const Zombie* z){return IsRunner(z)&&!z->mDead&&z->mHasHead&&int(z->mZombiePhase)>=RunIn&&int(z->mZombiePhase)<=RunOut;}
void Reset(){identities.clear();dodged.clear();}
void Forget(Zombie* z){identities.erase(z);for(auto& [shot,targets]:dodged)targets.erase(z);}
bool Restore(Zombie* z,int id){
 const auto* d=Find(id);if(!z||z->mDead||!d||int(z->mZombieType)!=d->base)return false;
 identities[z]=id;
 // mHasHead is also the native alive/targetable/eating gate. Do not set it
 // false at spawn, heal it on load, or change a wounded zombie's saved state.
 if(id==Louis)z->SetupReanimForLostHead();
 if(IsClever(z)){RefreshCleverRig(z);z->UpdateAnimSpeed();}
 if(id==Runner&&IsRunning(z)){
  if(int(z->mZombiePhase)==RunBrake)z->mPhaseCounter=std::min(z->mPhaseCounter,BrakeTicks);
  z->UpdateAnimSpeed();
 }
 return true;
}
void Assign(Zombie* z,int id){
 if(!Restore(z,id))return;
 if(IsClever(z)){
  // These boss-only fields are unused on normal/cone rigs and already saved
  // by the native serializer. No Zombie struct/legacy-save ABI changes.
  // Stomp = dodge recovery; Head = carried logical seed + 1; Bungee = source row.
  z->mBossStompCounter=0;z->mBossHeadCounter=0;z->mBossBungeeCounter=z->mRow;
  z->mTargetRow=z->mRow;z->UpdateAnimSpeed();return;
 }
 if(id==ConeWrap){
  z->mBodyHealth=z->mBodyMaxHealth=Find(id)->health;
  z->mHelmHealth=z->mHelmMaxHealth=Find(id)->armor;
  z->UpdateAnimSpeed();return;
 }
 if(id!=Runner)return;
 // These native fields are already serialized. Restore() deliberately does
 // not reset them: a saved fleeing runner must never charge in a second time.
 z->mZombiePhase=static_cast<decltype(z->mZombiePhase)>(RunIn);
 z->mPhaseCounter=0;z->mTargetCol=-1;z->mHasObject=false;z->StopEating();
 z->PlayZombieReanim("anim_walk2",REANIM_LOOP,0,RunAnimRate);
}
bool UpdateRunner(Zombie* z){
 if(!IsRunner(z)||!z->mBoard||!z->IsOnBoard()||z->IsDeadOrDying())return false;
 if(!z->mHasHead){
  if(int(z->mZombiePhase)>=RunIn&&int(z->mZombiePhase)<=RunOut){z->mZombiePhase=PHASE_ZOMBIE_NORMAL;z->StartWalkAnim(10);}
  return false; // Native injury/death, never an invulnerable getaway.
 }
 if(!IsRunning(z))return false;
 z->StopEating();
 // Preserve native grounding gates too (e.g. a runner carried by a bungee).
 if(z->ZombieNotWalking())return true;
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
bool SkipsProjectile(const Projectile* shot,const Zombie* z){
 if(!shot||!IsClever(z))return false;
 if(shot->mProjectileType==PROJECTILE_COBBIG||shot->mProjectileType==PROJECTILE_BASKETBALL||shot->mProjectileType==PROJECTILE_ZOMBIE_PEA)return false;
 const auto it=dodged.find(shot);
 if(it!=dodged.end()&&it->second.contains(z))return true;
 return IsDodging(z)&&z->mIceTrapCounter==0&&z->mButteredCounter==0;
}
bool DodgeProjectile(Projectile* shot,Zombie* z){
 if(!shot||!IsClever(z)||z->IsDeadOrDying())return false;
 if(SkipsProjectile(shot,z))return true;
 if(!z->mHasHead||!z->IsOnBoard()||z->IsImmobilizied()||z->mMindControlled||
    z->mZombieHeight!=HEIGHT_ZOMBIE_NORMAL||z->mBossStompCounter>0||
    (int(z->mZombiePhase)!=PHASE_ZOMBIE_NORMAL&&int(z->mZombiePhase)!=CleverFlee))return false;
 // Dodge plant projectiles, not explosions, mower/crush, or enemy shots.
 if(shot->mProjectileType==PROJECTILE_COBBIG||shot->mProjectileType==PROJECTILE_BASKETBALL||shot->mProjectileType==PROJECTILE_ZOMBIE_PEA)return false;
 if(Sexy::Rand(100)>=50)return false;
 dodged[shot].insert(z);z->StopEating();
 z->mZombiePhase=static_cast<decltype(z->mZombiePhase)>(CleverFlip);z->mPhaseCounter=FlipTicks;
 z->mBossBungeeCounter=z->mRow;z->mTargetRow=z->mRow;
 if(Sexy::Rand(100)<25){
  int choices[2],count=0;
  for(int row:{z->mRow-1,z->mRow+1})if(z->mBoard->RowCanHaveZombies(row))choices[count++]=row;
  if(count)z->mTargetRow=choices[Sexy::Rand(count)];
 }
 z->PlayZombieReanim("anim_walk2",REANIM_LOOP,0,0);z->UpdateReanim();return true;
}
void RefreshCleverRig(Zombie* z){
 if(!IsClever(z))return;
 auto* body=gLawnApp->ReanimationTryToGet(z->mBodyReanimID);if(!body)return;
 if(z->mHasHead&&body->TrackExists("anim_head1"))body->SetImageOverride("anim_head1",SandboxArt::NativeImage("Zombie_head_sunglasses1.png"));
 const bool ring=z->mInPool;
 z->ReanimShowPrefix("Zombie_duckytube",ring?RENDER_GROUP_NORMAL:RENDER_GROUP_HIDDEN);
 if(ring){z->ReanimIgnoreClipRect("Zombie_duckytube",true);z->SetupWaterTrack("Zombie_whitewater");z->SetupWaterTrack("Zombie_whitewater2");}
}
bool UpdateClever(Zombie* z){
 if(!IsClever(z)||!z->IsOnBoard()||z->IsDeadOrDying())return false;
 if(!z->mHasHead||z->mMindControlled){
  if(int(z->mZombiePhase)==CleverFlip||int(z->mZombiePhase)==CleverFlee){
   z->mZombiePhase=PHASE_ZOMBIE_NORMAL;z->mPhaseCounter=0;z->mBossHeadCounter=0;
   z->mAltitude=z->mInPool?-40*z->mScaleZombie:0;z->StartWalkAnim(0);
  }
  return false;
 }
 if(z->IsImmobilizied())return IsDodging(z);
 if(!IsDodging(z)){if(z->mBossStompCounter>0)--z->mBossStompCounter;return false;}
 const float previous=1-float(z->mPhaseCounter)/FlipTicks;
 if(z->mPhaseCounter>0)--z->mPhaseCounter;
 const float t=1-float(z->mPhaseCounter)/FlipTicks;
 const int from=z->mBossBungeeCounter,to=z->mTargetRow;
 if(!z->mBoard->RowCanHaveZombies(from)||!z->mBoard->RowCanHaveZombies(to)){
  z->mBossBungeeCounter=z->mTargetRow=z->mRow;z->mPhaseCounter=0;
 }else{
  z->mPosY=z->GetPosYBasedOnRow(from)*(1-t)+z->GetPosYBasedOnRow(to)*t;
  if(t>=.5f&&z->mRow!=to)z->SetRow(to);
 }
 z->mPosX+=(IsRetreating(z)?-28:28)*(t-previous);z->mX=int(z->mPosX);z->mY=int(z->mPosY);
 const bool pool=z->mBoard->IsPoolSquare(z->mBoard->PixelToGridXKeepOnBoard(z->mX+60,z->mY),z->mRow)&&z->mPosX<680;
 z->mInPool=pool;
 const float fromDepth=z->mBoard->IsPoolSquare(0,from)&&z->mPosX<680?-40*z->mScaleZombie:0;
 const float toDepth=z->mBoard->IsPoolSquare(0,to)&&z->mPosX<680?-40*z->mScaleZombie:0;
 z->mAltitude=fromDepth*(1-t)+toDepth*t+55*std::sin(3.14159265f*t);
 if(z->mPhaseCounter==0){
  z->mZombiePhase=static_cast<decltype(z->mZombiePhase)>(z->mBossHeadCounter>0?CleverFlee:int(PHASE_ZOMBIE_NORMAL));
  z->mBossStompCounter=DodgeRecovery;z->mAltitude=pool?-40*z->mScaleZombie:0;
  z->mPosY=z->GetPosYBasedOnRow(z->mRow);z->mY=int(z->mPosY);z->StartWalkAnim(0);
 }
 RefreshCleverRig(z);z->CheckForBoardEdge();return true;
}
bool StealPlant(Zombie* z,Plant* p){
 if(!IsClever(z)||!p||p->mDead||p->NotOnGround()||IsDodging(z)||z->mBossHeadCounter>0||!z->mHasHead||z->mMindControlled)return false;
 // Steal exactly the native melee target; don't erase the whole stacked cell.
 z->mBossHeadCounter=SandboxPlants::Type(p)+1;
 p->Die();z->StopEating();z->mZombiePhase=static_cast<decltype(z->mZombiePhase)>(CleverFlee);
 z->mPhaseCounter=0;z->StartWalkAnim(0);return true;
}
void DrawCarriedPlant(Sexy::Graphics* g,Zombie* z){
 if(!IsClever(z)||z->mBossHeadCounter<=0||!z->mHasHead||z->IsDeadOrDying())return;
 const int seed=SandboxPlants::Base(z->mBossHeadCounter-1);if(seed<0||seed>=54)return;
 auto* body=gLawnApp->ReanimationTryToGet(z->mBodyReanimID);if(!body)return;
 static std::map<int,std::unique_ptr<Sexy::MemoryImage>> plants;
 auto& image=plants[seed];if(!image)image=gLawnApp->mReanimatorCache->MakeCachedPlantFrame(static_cast<SeedType>(seed),VARIATION_NORMAL);
 if(!image)return;
 auto m=body->mOverlayMatrix;
 const float carryY=z->mInPool?52.0f:70.0f;
 m.m02+=m.m00*15+m.m01*carryY+g->mTransX;m.m12+=m.m10*15+m.m11*carryY+g->mTransY;
 m.m00*=.5f;m.m01*=.5f;m.m10*=.5f;m.m11*=.5f;
 PvzpBltMatrix(g,image.get(),m,g->mClipRect,Sexy::Color(255,255,255),g->mDrawMode,Sexy::Rect(0,0,image->mWidth,image->mHeight));
}
void Tick(Board*){}
bool HasInteraction(const Zombie* z){return LegacyPhase(z)||IsRunning(z)||IsDodging(z);}
bool IsHeld(const Zombie*){return false;}
bool UpdateInteraction(Zombie* z){ReleaseLegacy(z);return UpdateClever(z);}
bool CatchForReturn(Plant*,Zombie*){return false;}
bool Staple(Zombie*){return false;}
bool Slip(Zombie*){return false;}
bool Misdirect(Zombie*){return false;}
bool IsRetreating(Zombie* z){return (IsRunner(z)&&z->mHasObject)||(IsClever(z)&&z->mBossHeadCounter>0&&z->mHasHead);}
bool IsFeigning(Zombie*){return false;}
bool IsResting(Zombie*){return false;}
void PoleLanded(Zombie*){}
void ArmorBroken(Zombie*){}
void AdjustPose(Zombie* z,Reanimation* body){
 if(body&&IsDodging(z)){
  const float t=1.0f-float(z->mPhaseCounter)/FlipTicks;
  // Fast takeoff/landing, stretched mid-air turn: local bullet-time only.
  const float p=t<.2f?t*1.5f:t<.8f?.3f+(t-.2f)*(.4f/.6f):.7f+(t-.8f)*1.5f;
  // The native overlay already mirrors a fleeing zombie. Reversing the
  // angle a second time would turn its backflip into a forward somersault.
  const float angle=p*6.2831853f;
  const float c=std::cos(angle),s=std::sin(angle);
  auto& m=body->mOverlayMatrix;const auto old=m;
  m.m00=old.m00*c+old.m01*s;m.m01=-old.m00*s+old.m01*c;
  m.m10=old.m10*c+old.m11*s;m.m11=-old.m10*s+old.m11*c;
  m.m02=old.m02+old.m00*40+old.m01*80-m.m00*40-m.m01*80;
  m.m12=old.m12+old.m10*40+old.m11*80-m.m10*40-m.m11*80;
  return;
 }
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
 static std::array<std::unique_ptr<Sexy::MemoryImage>,Definitions.size()> portraits;
 auto& portrait=portraits[Find(id)-Definitions.data()];
 if(!portrait){
  portrait=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(200,210);Sexy::Graphics canvas(portrait.get());canvas.SetLinearBlend(true);
  Reanimation anim;anim.ReanimationInitializeType(40,40,id==GiantImp?REANIM_IMP:REANIM_ZOMBIE);anim.SetFramesForLayer(id==GiantImp?"anim_walk":id==Runner?"anim_walk2":"anim_idle");Zombie::SetupReanimLayers(&anim,static_cast<ZombieType>(Base(id)));
  if(id==Louis)for(const char* prefix:{"anim_head","anim_hair","anim_tongue"})anim.AssignRenderGroupToPrefix(prefix,RENDER_GROUP_HIDDEN);
  if(id==Clever||id==CleverCone)anim.SetImageOverride("anim_head1",SandboxArt::NativeImage("Zombie_head_sunglasses1.png"));
  if(id==Runner){anim.mAnimTime=0.35f;anim.mOverlayMatrix.m01=0.13f;anim.mOverlayMatrix.m02-=15.6f;}
  AbstractRigVisuals::Scope pose(&anim,id);anim.Draw(&canvas);
 }
 SandboxArt::DrawFit(g,portrait.get(),x+4,y+5,w-8,h-10);
}
float Speed(Zombie* z){return IsClever(z)?(IsRetreating(z)?CleverFleeSpeed:CleverSpeed):IsConeWrap(z)?ConeWrapSpeed:1.0f;}
int Damage(Zombie*,int damage,unsigned){return damage;}
bool ElectricHit(Zombie*){return false;}
void CombatDeath(Zombie*){}
void DrawEffects(Sexy::Graphics*,Board*,int){}
bool HasShot(const Projectile*){return false;}
bool DrawShot(Sexy::Graphics*,const Projectile*){return false;}
bool Impact(Projectile*,Plant*){return false;}
Plant* CollisionTarget(Projectile*){return nullptr;}
void ForgetShot(Projectile* shot){dodged.erase(shot);}
void ForgetPlant(Plant*){}
bool AttackSlowed(const Plant*){return false;}
void RefreshDamageArt(Zombie* z){if(IsLouis(z))z->SetupReanimForLostHead();if(IsClever(z))RefreshCleverRig(z);}
Sexy::Image* DetachedArmor(const Zombie*){return nullptr;}
Sexy::Image* DetachedHead(const Zombie* z){return IsGiantImp(z)?SandboxArt::NativeImage("Zombie_gargantuar_head.png"):nullptr;}
}
