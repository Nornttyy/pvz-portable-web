#include "IceChili.h"
#include "IceChiliRules.h"
#include "MemeCharacters.h"
#include "LawnApp.h"
#include "Lawn/Plant.h"
#include "Lawn/Board.h"
#include "Lawn/Zombie.h"
#include <algorithm>
namespace IceChili {
bool Detonate(Plant* p){
 if(!p||p->mDead||MemeCharacters::Type(p)!=IceChiliRules::Id||!p->mBoard)return false;
 auto* b=p->mBoard;
 for(auto* z:b->mZombies){
  if(z->mDead||!z->IsOnBoard()||z->mMindControlled||z->IsDeadOrDying()||z->mRow!=p->mRow||!z->EffectedByDamage(127))continue;
  z->TakeDamage(IceChiliRules::Damage,1U);
  if(z->IsDeadOrDying()||!z->CanBeFrozen())continue;
  // Native ice blocks, paused movement/attacks and thaw. No hidden extra
  // Ice-shroom damage or twenty-second slow; existing longer ice is retained.
  z->mIceTrapCounter=std::max(z->mIceTrapCounter,IceChiliRules::Freeze);
  z->StopZombieSound();
  if(z->mZombieType==ZOMBIE_BALLOON)z->BalloonPropellerHatSpin(false);
  z->UpdateAnimSpeed();
 }
 b->mApp->PlayFoley(FOLEY_FROZEN);
 // Original ice shards span this lane, not a full-screen flash or blue fire.
 for(int col=0;col<9;++col){const float x=b->GridToPixelX(col,p->mRow)+40;
  b->mApp->AddPvzpParticle(x,b->GetPosYBasedOnRow(x,p->mRow)+60,Board::MakeRenderOrder(RENDER_LAYER_PARTICLE,p->mRow,1),PARTICLE_ICE_TRAP_RELEASE);
 }
 b->ShakeBoard(2,-2);p->Die();return true;
}
}
