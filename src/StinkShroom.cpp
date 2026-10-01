#include "StinkShroom.h"
#include "StinkShroomRules.h"
#include "MemeCharacters.h"
#include "SandboxArt.h"
#include "LawnApp.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include <map>
#include <cmath>
namespace StinkShroom {
namespace {
struct Status {int stun=0,push=0;bool flee=false;float step=0;};
std::map<const Plant*,int> exposures;
std::map<const Zombie*,Status> statuses;
Save pending;
int hitCounts[4]{},lastYuck=-1000;
bool Source(const Plant* p){return !p->mDead&&!p->mSquished&&!p->mIsAsleep&&p->mPlantHealth>0&&!const_cast<Plant*>(p)->NotOnGround()&&MemeCharacters::Type(p)==StinkShroomRules::Id;}
bool Nearby(const Plant* p){
 if(!p->mBoard||p->mDead||p->mSquished||MemeCharacters::Type(p)==StinkShroomRules::Id||const_cast<Plant*>(p)->NotOnGround())return false;
 for(auto* other:p->mBoard->mPlants)if(other!=p&&Source(other)&&StinkShroomRules::Adjacent(p->mPlantCol,p->mRow,other->mPlantCol,other->mRow))return true;
 return false;
}
}
void Reset(){exposures.clear();statuses.clear();pending={};std::fill(std::begin(hitCounts),std::end(hitCounts),0);lastYuck=-1000;}
void Forget(Plant* p){exposures.erase(p);}
void Forget(Zombie* z){statuses.erase(z);}
void UpdatePlant(Plant* p){
 if(!p->mBoard||p->mBoard->mPaused)return;
 if(Nearby(p))exposures[p]=StinkShroomRules::ExposureTick(exposures[p],true);else exposures.erase(p);
}
bool Affected(const Plant* p){auto i=exposures.find(p);return i!=exposures.end()&&i->second>=StinkShroomRules::Exposure&&Nearby(p);}
bool WorkTick(const Plant* p){return !Affected(p)||StinkShroomRules::WorkTick(StinkShroomRules::Exposure,p->mBoard->mMainCounter);}
bool Stunned(const Zombie* z){auto i=statuses.find(z);return i!=statuses.end()&&i->second.stun>0;}
bool Fleeing(const Zombie* z){auto i=statuses.find(z);return i!=statuses.end()&&i->second.flee;}
bool Controls(const Zombie* z){auto i=statuses.find(z);return i!=statuses.end()&&(i->second.stun||i->second.push||i->second.flee);}
void Hit(Plant* p,Zombie* z){
 if(MemeCharacters::Type(p)!=StinkShroomRules::Id||z->mDead||!z->IsOnBoard()||z->IsDeadOrDying()||z->mMindControlled)return;
 // Native stationary boss/bungee and airborne transitions are not ground walkers.
 if(z->mZombieType==ZOMBIE_BOSS||z->mZombieType==ZOMBIE_BUNGEE||z->mZombieHeight!=HEIGHT_ZOMBIE_NORMAL)return;
 const auto outcome=StinkShroomRules::Roll(Sexy::Rand(100));++hitCounts[outcome];
 if(outcome==StinkShroomRules::Normal)return;
 auto& s=statuses[z];
 if(outcome==StinkShroomRules::Flee){
  s={0,0,true,0};z->mZombiePhase=PHASE_ZOMBIE_NORMAL;z->mPhaseCounter=0;
  z->mAltitude=z->mInPool?-40.f:0.f;z->mTargetRow=z->mRow;z->StopEating();z->StartWalkAnim(5);
 }else if(!s.flee){
  if(outcome==StinkShroomRules::Stun)s.stun=StinkShroomRules::StunTicks;
  else{const bool giant=z->mZombieType==ZOMBIE_GARGANTUAR||z->mZombieType==ZOMBIE_REDEYE_GARGANTUAR;
   s.push=StinkShroomRules::PushTicks;s.step=(giant?StinkShroomRules::PushGiant:StinkShroomRules::PushNormal)/s.push;}
  z->StopEating();
 }
 z->UpdateAnimSpeed();
 if(int(p->mBoard->mMainCounter)-lastYuck>=30){lastYuck=p->mBoard->mMainCounter;p->mApp->PlayFoley(FOLEY_YUCK);}
}
bool UpdateZombie(Zombie* z){
 auto i=statuses.find(z);if(i==statuses.end())return false;
 if(z->mDead||z->IsDeadOrDying()||z->mMindControlled){statuses.erase(i);z->UpdateAnimSpeed();return false;}
 if(z->mBoard->mPaused)return true;
 auto& s=i->second;
 // Nausea lasts exactly half a second; existing ice/butter retain their own clocks.
 if(s.stun>0){--s.stun;if(!s.stun)z->UpdateAnimSpeed();return true;}
 if(z->mIceTrapCounter>0||z->mButteredCounter>0)return true;
 if(s.flee){z->StopEating();z->ApplyAnimRate(60.f);z->mPosX+=StinkShroomRules::FleeSpeed*(z->IsMovingAtChilledSpeed()?.5f:1.f);z->mX=int(z->mPosX);z->mPosY=z->GetPosYBasedOnRow(z->mRow);z->mY=int(z->mPosY);z->CheckForBoardEdge();return true;}
 if(s.push>0){z->mPosX+=s.step;z->mX=int(z->mPosX);z->mPosY=z->GetPosYBasedOnRow(z->mRow);z->mY=int(z->mPosY);--s.push;return true;}
 statuses.erase(i);return false;
}
Save Capture(Board* b){Save out;
 for(auto* p:b->mPlants)if(!p->mDead){auto i=exposures.find(p);if(i!=exposures.end())out.plants.push_back({b->mPlants.DataArrayGetID(p),i->second});}
 for(auto* z:b->mZombies)if(!z->mDead){auto i=statuses.find(z);if(i!=statuses.end()){const auto& s=i->second;out.zombies.push_back({b->mZombies.DataArrayGetID(z),s.stun,s.push,int(s.flee),int(std::lround(s.step*1000))});}}
 return out;
}
void Load(const Save& save){pending=save;}
void Restore(Board* b){
 for(const auto& record:pending.plants)if(record.exposure>0&&record.exposure<=StinkShroomRules::Exposure)if(auto* p=b->mPlants.DataArrayTryToGet(record.key);p&&!p->mDead)exposures[p]=record.exposure;
 for(const auto& record:pending.zombies)if(record.stun>=0&&record.stun<=50&&record.push>=0&&record.push<=20&&(record.flee==0||record.flee==1)&&record.stepMilli>=0&&record.stepMilli<=2000)
  if(auto* z=b->mZombies.DataArrayTryToGet(record.key);z&&!z->mDead){statuses[z]={record.stun,record.push,bool(record.flee),record.stepMilli/1000.f};z->UpdateAnimSpeed();}
 pending={};
}
void DrawEffects(Sexy::Graphics* g,Board* b,int row){
 auto* puff=SandboxArt::NativeImage("puff_3.png");if(!puff)return;
 auto odor=[&](float x,float y){for(int i=0;i<2;++i){const float t=((b->mMainCounter+i*29)%60)/60.f;
  Sexy::Graphics draw(*g);draw.SetColorizeImages(true);draw.SetColor(Sexy::Color(125,103,57,int(125*(1-t))));
  PvzpDrawImageScaledF(&draw,puff,x-4+i*12+std::sin(t*5)*3,y-t*18,(12+6*t)/puff->mWidth,(12+6*t)/puff->mHeight);}};
 for(auto* z:b->mZombies)if(!z->mDead&&!z->IsDeadOrDying()&&z->IsOnBoard()&&z->mRow==row&&Stunned(z))odor(z->mPosX+47,z->mPosY-z->mAltitude+5);
}
}
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_stink_data(int kind,int index,int field){
 auto* b=gLawnApp?gLawnApp->mBoard:nullptr;if(!b)return -1;
 if(kind==2)return field>=0&&field<4?StinkShroom::hitCounts[field]:-1;
 if(kind==3)return field==0?b->mMainCounter:field==1?b->mPaused:-1;
 if(index<0)return -1;
 if(kind==0){for(auto* p:b->mPlants)if(!p->mDead)if(index--==0){auto i=StinkShroom::exposures.find(p);return field==0?(i==StinkShroom::exposures.end()?0:i->second):field==1?StinkShroom::Affected(p):field==2?MemeCharacters::Type(p):field==3?p->mPlantCol:field==4?p->mRow:field==5?p->mLaunchCounter:field==6?p->mShootingCounter:-1;}}
 if(kind==1){for(auto* z:b->mZombies)if(!z->mDead&&z->IsOnBoard())if(index--==0){auto i=StinkShroom::statuses.find(z);const StinkShroom::Status s=i==StinkShroom::statuses.end()?StinkShroom::Status{}:i->second;return field==0?s.stun:field==1?s.push:field==2?s.flee:field==3?int(z->mPosX*1000):field==4?z->mRow:-1;}}
 return -1;
}
#endif
