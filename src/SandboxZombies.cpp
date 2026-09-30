// Abstract newspaper replacement uses native phase/health/save fields and rig.
#include "SandboxZombies.h"
#include "SandboxArt.h"
#include "MemeCharacters.h"
#include "LawnApp.h"
#include "Lawn/Zombie.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "PvzpLib/Reanimator.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include <algorithm>
#include <cmath>
extern bool gSandboxEnabled;
namespace SandboxZombies {
namespace {
bool Enabled(){return gSandboxEnabled||(gLawnApp&&gLawnApp->IsAdventureMode());}
bool Walker(Zombie* z){return z&&z->IsOnBoard()&&!z->mDead&&!z->IsDeadOrDying()&&z->mHasHead&&!z->mMindControlled&&z->mZombiePhase==PHASE_ZOMBIE_NORMAL&&z->mZombieHeight==HEIGHT_ZOMBIE_NORMAL&&!z->mInPool;}
bool Portable(Zombie* z){if(!Walker(z))return false;const int t=int(z->mZombieType);return t==0||t==1||t==2||t==4||t==6||t==7||t==24;}
void Phase(Zombie* z,int phase,int ticks){z->StopEating();z->mZombiePhase=static_cast<decltype(z->mZombiePhase)>(phase);z->mPhaseCounter=ticks;}
void Land(Zombie* z){
 Phase(z,int(z->mZombieType)==21&&z->mShieldHealth>0?PHASE_LADDER_CARRYING:PHASE_ZOMBIE_NORMAL,0);
 z->mTargetPlantID=static_cast<decltype(z->mTargetPlantID)>(0);z->mTargetRow=-1;
 z->mAltitude=0;z->mZombieHeight=HEIGHT_ZOMBIE_NORMAL;z->mPosY=z->GetPosYBasedOnRow(z->mRow);z->mY=int(z->mPosY);z->StartWalkAnim(10);
}
bool Ready(Zombie* z){return Portable(z)&&!IsFeigning(z)&&!IsResting(z)&&!z->mIceTrapCounter&&!z->mButteredCounter;}
bool DryRow(Board* b,int row){return row>=0&&row<(b->StageHasPool()?6:5)&&b->RowCanHaveZombies(row)&&!(b->StageHasPool()&&(row==2||row==3));}
// Advance in small steps but never skip a plant, pumpkin or the mower/house edge.
// Native eating resumes after landing. Ground spikes and support pots are not walls.
float ForwardLimit(Zombie* z,float desired){
 float stop=std::max(95.0f,desired);
 for(auto* p:z->mBoard->mPlants)if(!p->mDead&&!p->mSquished&&p->mPlantHealth>0&&p->mRow==z->mRow){
  const int type=int(p->mSeedType);if(type==16||type==33||type==21||type==46)continue;
  const float right=p->mX+(type==47?145:65)-z->mZombieAttackRect.mX;
  if(p->mX<z->mPosX+z->mZombieAttackRect.mX+20&&right>stop)stop=std::min(z->mPosX,right);
 }
 return std::min(z->mPosX,stop);
}
void TripNeighbors(Zombie* z,float reach){
 for(auto* other:z->mBoard->mZombies)if(other!=z&&Ready(other)&&other->mRow==z->mRow&&std::abs(other->mPosX-z->mPosX)<reach){Phase(other,Tripped,100);gLawnApp->PlayMemeCue(2,3);}
}
void DropPassenger(Zombie* z){
 const int height=std::max(0,int(z->mAltitude));Phase(z,AirDrop,45);z->mTargetCol=height;z->mTargetPlantID=static_cast<decltype(z->mTargetPlantID)>(0);
}
}
bool HasInteraction(const Zombie* z){return z&&int(z->mZombiePhase)>=Held&&int(z->mZombiePhase)<=AirDrop;}
bool IsHeld(const Zombie* z){return z&&int(z->mZombiePhase)==Held;}
bool CatchForReturn(Plant* p,Zombie* z){
 if(!p||p->mDead||p->mSquished||p->mPlantHealth<=0||MemeCharacters::Type(p)!=509||!Portable(z))return false;
 for(auto* other:p->mBoard->mZombies)if(IsHeld(other)&&other->mTargetCol==p->mPlantCol&&other->mRow==p->mRow)return false;
 Phase(z,Held,100);z->mTargetCol=p->mPlantCol;z->mPosX=p->mX+45;z->mX=int(z->mPosX);z->mAltitude=0;
 z->mTargetPlantID=static_cast<decltype(z->mTargetPlantID)>(p->mBoard->mPlants.DataArrayGetID(p));
 p->mState=STATE_CHOMPER_DIGESTING;p->mStateCountdown=100;p->PlayBodyReanim("anim_chew",REANIM_LOOP,5,18);return true;
}
bool Staple(Zombie* z){
 if(!Portable(z))return false;Zombie* neighbor=nullptr;
 for(auto* q:z->mBoard->mZombies)if(q!=z&&Portable(q)&&q->mRow==z->mRow&&q->mPosX>z->mPosX+10&&q->mPosX<z->mPosX+130&&(!neighbor||q->mPosX<neighbor->mPosX))neighbor=q;
 if(!neighbor)return false; // One isolated zombie cannot be stapled to itself.
 const int anchor=int((z->mPosX+neighbor->mPosX)*.5f);
 for(auto* q:{z,neighbor}){Phase(q,Pinned,180);q->mTargetCol=anchor;}
 gLawnApp->PlayMemeCue(3,-4);return true;
}
bool Slip(Zombie* z){if(!Portable(z))return false;Phase(z,Slipping,80);z->mTargetCol=int(z->mPosX);gLawnApp->PlayMemeCue(2,-4);return true;}
bool Misdirect(Zombie* z){if(!Portable(z))return false;Phase(z,Misdirected,130);gLawnApp->PlayMemeCue(1,5);return true;}
bool UpdateInteraction(Zombie* z){
 if(!HasInteraction(z))return false;
 if(z->mBoard->mPaused)return true;
 if(z->mDead||z->IsDeadOrDying()||!z->mHasHead||z->mMindControlled){Land(z);return false;}
 const int phase=int(z->mZombiePhase);
 if(phase==Held){
  Plant* owner=z->mBoard->mPlants.DataArrayTryToGet(static_cast<unsigned>(z->mTargetPlantID));
  if(owner&&(owner->mDead||owner->mSquished||owner->mPlantHealth<=0||MemeCharacters::Type(owner)!=509))owner=nullptr;
  if(!owner){Land(z);return true;} // Shovel/crush/death releases the SAME living zombie.
  if(z->mPhaseCounter>0)--z->mPhaseCounter;
  if(z->mPhaseCounter==0){
   Phase(z,Returned,70);z->mTargetCol=int(z->mPosX);owner->mState=STATE_CHOMPER_DIGESTING;owner->mStateCountdown=650;owner->PlayBodyReanim("anim_bite",REANIM_PLAY_ONCE_AND_HOLD,5,30);gLawnApp->PlayMemeCue(0,6);
  }
  return true;
 }
 if(phase==Airlift){
  // A generation-bearing ID in a phase-tagged, serialized slot, NOT a native
  // related-zombie link: killing/charming a passenger must not kill the carrier.
  auto* carrier=z->mBoard->mZombies.DataArrayTryToGet(static_cast<unsigned>(z->mTargetPlantID));
  if(!carrier||carrier->mDead||carrier->IsDeadOrDying()||!carrier->IsOnBoard()||!carrier->mHasHead||carrier->mMindControlled||carrier->mBlowingAway||carrier->mPosX<95||carrier->mPosX>780||int(carrier->mZombieType)!=16||carrier->mZombiePhase!=PHASE_BALLOON_FLYING||!DryRow(z->mBoard,carrier->mRow)){DropPassenger(z);return true;}
  if(z->mIceTrapCounter||z->mButteredCounter){DropPassenger(z);return true;}
  if(!carrier->mIceTrapCounter&&!carrier->mButteredCounter&&z->mPhaseCounter>0)--z->mPhaseCounter;
  const float lift=std::min(1.0f,(150-z->mPhaseCounter)/30.0f);
  z->mPosX=z->mTargetCol+(carrier->mPosX+75-z->mTargetCol)*lift;z->mPosY=z->GetPosYBasedOnRow(z->mRow);
  z->mAltitude=35*lift;z->mX=int(z->mPosX);z->mY=int(z->mPosY);
  if(!z->mPhaseCounter)DropPassenger(z);
  return true;
 }
 // Pin/flight clocks and movement freeze together. Butter stays attached while sliding.
 if(z->mIceTrapCounter>0||(z->mButteredCounter>0&&phase!=Slipping))return true;
 if(z->mPhaseCounter>0)--z->mPhaseCounter;
 if(phase==Returned||phase==Slipping){
  const float total=phase==Returned?70.0f:80.0f,t=1-z->mPhaseCounter/total;
  z->mPosX=std::min(780.0f,z->mTargetCol+(phase==Returned?210:155)*t);
  z->mPosY=z->GetPosYBasedOnRow(z->mRow);z->mAltitude=phase==Returned?62*std::sin(t*3.14159265f):0;
  for(auto* other:z->mBoard->mZombies)if(other!=z&&Portable(other)&&other->mRow==z->mRow&&std::abs(other->mPosX-z->mPosX)<26){Phase(other,Tripped,100);gLawnApp->PlayMemeCue(2,3);}
 }else if(phase==Misdirected){z->mPosX=std::min(780.0f,z->mPosX+.65f);}
 else if(phase==Hurried||phase==BrakeSlide){
  const float total=phase==Hurried?60.0f:70.0f,t=1-z->mPhaseCounter/total;
  const float desired=z->mTargetCol-(phase==Hurried?105:110)*t*(2-t);
  z->mPosX=ForwardLimit(z,desired);z->mAltitude=phase==Hurried?35*std::sin(t*3.14159265f):0;
  if(phase==BrakeSlide)TripNeighbors(z,28);
  if(z->mPosX>desired+.1f||!z->mPhaseCounter){
   const bool skid=phase==BrakeSlide;Land(z);if(skid)Phase(z,Tripped,100);
  }
 }else if(phase==DoorDash){
  if(!z->mShieldHealth){Land(z);return true;}
  const float desired=z->mPosX+(z->mTargetCol-z->mPosX)/(z->mPhaseCounter+1.0f);
  z->mPosX=ForwardLimit(z,desired);
  if(z->mPosX>desired+.1f)Land(z);
 }else if(phase==LaneStep){
  if(!DryRow(z->mBoard,z->mTargetCol)||!DryRow(z->mBoard,z->mTargetRow)){Land(z);return true;}
  const float t=1-z->mPhaseCounter/45.0f,ease=t*t*(3-2*t);
  const float from=z->GetPosYBasedOnRow(z->mTargetCol),to=z->GetPosYBasedOnRow(z->mTargetRow);
  if(t>=.5f&&z->mRow!=z->mTargetRow)z->SetRow(z->mTargetRow);
  z->mPosY=from+(to-from)*ease;
 }else if(phase==AirDrop){
  const float t=1-z->mPhaseCounter/45.0f;z->mAltitude=z->mTargetCol*(1-t*t);
 }
 z->mX=int(z->mPosX);z->mY=int(z->mPosY);
 if(!z->mPhaseCounter)Land(z);
 return true;
}
bool IsRetreating(Zombie* z){return Enabled()&&Walker(z)&&int(z->mZombieType)==24&&z->mZombieAge%600>=400&&z->mZombieAge%600<500&&z->mPosX<760;}
// The normal cone's native phase counter is serialized with the zombie. Only
// a real armor break starts this one-time gag; hits never push or restart it.
bool IsFeigning(Zombie* z){return Enabled()&&Walker(z)&&int(z->mZombieType)==2&&z->mHelmHealth==0&&z->mPhaseCounter>0&&z->mPhaseCounter<=300;}
bool IsResting(Zombie* z){
 if(!Enabled()||!Walker(z))return false;
 const int age=z->mZombieAge%800,type=int(z->mZombieType);
 const int flagAge=z->mZombieAge%600;
 return (type==0&&age>=600)||(type==1&&flagAge>=270&&flagAge<330)||(type==4&&z->mPhaseCounter>0&&z->mPhaseCounter<=40);
}
void PoleLanded(Zombie* z){if(Enabled()&&z&&int(z->mZombieType)==3)z->mPhaseCounter=800;}
void ArmorBroken(Zombie* z){
 if(!Enabled()||!Walker(z)||int(z->mZombieType)!=2||z->mHelmHealth!=0)return;
 z->mPhaseCounter=300;z->StopEating();
}
void AdjustPose(Zombie* z,Reanimation* body){
 if(!body||!Enabled())return;
 float angle=0;
 const int interaction=int(z->mZombiePhase);
 if(interaction==Returned){angle=-1.25f*std::sin((70-z->mPhaseCounter)*3.14159265f/70);}
 else if(interaction==Tripped){angle=-1.42f*std::min(1.0f,std::min((100-z->mPhaseCounter)/12.0f,z->mPhaseCounter/25.0f));}
 else if(interaction==Slipping){angle=1.1f*std::sin((80-z->mPhaseCounter)*3.14159265f/80);}
 else if(interaction==Pinned){angle=.12f*std::sin(z->mPhaseCounter*.16f);}
 else if(interaction==Hurried){angle=-.40f*std::sin((60-z->mPhaseCounter)*3.14159265f/60);}
 else if(interaction==DoorDash){angle=-.32f*std::sin((45-z->mPhaseCounter)*3.14159265f/45);}
 else if(interaction==BrakeSlide){angle=.9f*std::sin((70-z->mPhaseCounter)*3.14159265f/140);}
 else if(interaction==Airlift){angle=.20f*std::sin((150-z->mPhaseCounter)*.10f);}
 else if(interaction==AirDrop){angle=.20f*std::sin(z->mPhaseCounter*3.14159265f/45);}
 else if(interaction==LaneStep){angle=(z->mTargetRow>z->mTargetCol?.20f:-.20f)*std::sin(z->mPhaseCounter*3.14159265f/45);}
 else if(interaction==Misdirected){auto& m=body->mOverlayMatrix;m.m02+=90*m.m00;m.m00=-m.m00;m.m10=-m.m10;return;}
 else if(IsRetreating(z)){
  angle=0.24f*std::sin((z->mZombieAge%600-400)*3.14159265f/100);
 }else if(int(z->mZombieType)==21&&z->mZombiePhase==PHASE_LADDER_CARRYING&&z->mPhaseCounter>0&&z->mPhaseCounter<=35){
  angle=0.18f*std::sin(z->mPhaseCounter*3.14159265f/35);
 }else if(IsFeigning(z)){
  const float t=z->mPhaseCounter>275?(300-z->mPhaseCounter)/25.0f:z->mPhaseCounter<40?z->mPhaseCounter/40.0f:1.0f;
  angle=-1.36f*t*t*(3-2*t);
 }else if(Walker(z)&&int(z->mZombieType)==4&&z->mPhaseCounter>0&&z->mPhaseCounter<=40){
  angle=-0.22f*std::sin(z->mPhaseCounter*3.14159265f/40);
 }else if(IsResting(z)&&int(z->mZombieType)==0){
  const int start=600,end=800,age=z->mZombieAge%800;
  const float fade=std::clamp(std::min(age-start,end-age)/20.0f,0.0f,1.0f);
  angle=-1.24f*fade+0.02f*fade*std::sin(age*.1f);
 }else if(IsResting(z)&&int(z->mZombieType)==1){angle=-.20f*std::sin((z->mZombieAge%600-270)*3.14159265f/60);}
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
 for(auto* z:b->mZombies){
  if(Walker(z)&&!z->mIceTrapCounter&&!z->mButteredCounter){
   const int type=int(z->mZombieType);
   if((type==0&&z->mZombieAge%800==630)||(IsFeigning(z)&&z->mPhaseCounter==275))TripNeighbors(z,65);
   if(type==1&&z->mHasArm&&z->mZombieAge%600==300){
    Zombie* follower=nullptr;
    for(auto* q:b->mZombies)if(q!=z&&Ready(q)&&!q->mIsEating&&q->mRow==z->mRow&&q->mPosX>z->mPosX+20&&q->mPosX<z->mPosX+170&&(!follower||q->mPosX<follower->mPosX))follower=q;
    if(follower){Phase(follower,Hurried,60);follower->mTargetCol=int(follower->mPosX);gLawnApp->PlayMemeCue(3,4);}
   }
   if(type==6&&z->mShieldHealth>0&&!z->mIsEating&&z->mZombieAge%600==300){
    Zombie* friendZ=nullptr;
    for(auto* q:b->mZombies)if(q!=z&&Ready(q)&&int(q->mZombieType)!=6&&q->mRow==z->mRow&&q->mPosX<z->mPosX-15&&q->mPosX>z->mPosX-120&&(!friendZ||q->mPosX<friendZ->mPosX))friendZ=q;
    if(friendZ){const float target=ForwardLimit(z,friendZ->mPosX-65);if(target<z->mPosX-20){Phase(z,DoorDash,45);z->mTargetCol=int(target);gLawnApp->PlayMemeCue(1,-3);}}
   }
   if(type==7&&!z->mIsEating&&z->mZombieAge%800==200){Phase(z,BrakeSlide,70);z->mTargetCol=int(z->mPosX);gLawnApp->PlayMemeCue(2,-3);}
  }
  if(Walker(z)&&int(z->mZombieType)==4&&z->mHasArm&&z->mHelmHealth>0&&!z->mIceTrapCounter&&!z->mButteredCounter&&((z->mZombieAge>0&&z->mZombieAge%600==0)||z->mPhaseCounter==20)){
   Zombie* victim=nullptr;
   for(auto* other:b->mZombies)if(other!=z&&Walker(other)&&(int(other->mZombieType)==0||int(other->mZombieType)==1||int(other->mZombieType)==2||int(other->mZombieType)==4||int(other->mZombieType)==6||int(other->mZombieType)==7||int(other->mZombieType)==24)&&other->mRow==z->mRow&&other->mPosX<z->mPosX&&other->mPosX>z->mPosX-100&&(!victim||other->mPosX>victim->mPosX))victim=other;
   if(victim){if(z->mPhaseCounter==20){victim->TakeDamage(40,0);gLawnApp->PlayMemeCue(1,-7);}else{z->mPhaseCounter=40;z->StopEating();}}
  }
  if(int(z->mZombieType)==3&&!z->mDead&&z->IsOnBoard()&&!z->IsDeadOrDying()&&z->mHasHead&&z->mHasArm&&!z->mMindControlled&&!z->mInPool&&z->mZombieHeight==HEIGHT_ZOMBIE_NORMAL&&z->mZombiePhase==PHASE_POLEVAULTER_POST_VAULT&&z->mPhaseCounter==0&&!z->mIceTrapCounter&&!z->mButteredCounter&&z->mPosX>100){
   z->StopEating();z->mHasObject=true;z->mZombiePhase=PHASE_POLEVAULTER_PRE_VAULT;z->mZombieAttackRect=Sexy::Rect(-29,0,70,115);
   for(const auto* part:{"Zombie_polevaulter_pole","Zombie_polevaulter_innerarm","Zombie_polevaulter_innerhand"})z->ReanimShowPrefix(part,RENDER_GROUP_NORMAL);
   z->PickRandomSpeed();z->PlayZombieReanim("anim_run",REANIM_LOOP,10,0);gLawnApp->PlayMemeCue(3,-6);
  }
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
   if(best!=z->mRow){z->mTargetCol=z->mRow;z->mTargetRow=best;Phase(z,LaneStep,45);gLawnApp->PlayMemeCue(2,-5);}
  }
  // Each balloon can lift one EXISTING walker once. No spawning/duplicate loot.
  if(!z->mDead&&z->IsOnBoard()&&!z->IsDeadOrDying()&&z->mHasHead&&!z->mMindControlled&&int(z->mZombieType)==16&&z->mZombiePhase==PHASE_BALLOON_FLYING&&!z->mIceTrapCounter&&!z->mButteredCounter&&!z->mSummonCounter&&z->mZombieAge%600==300&&z->mPosX>180&&z->mPosX<740&&DryRow(b,z->mRow)){
   Zombie* passenger=nullptr;
   for(auto* q:b->mZombies)if(q!=z&&Ready(q)&&!q->mIsEating&&q->mRow==z->mRow&&std::abs(q->mPosX-z->mPosX)<95&&(!passenger||std::abs(q->mPosX-z->mPosX)<std::abs(passenger->mPosX-z->mPosX)))passenger=q;
   if(passenger){Phase(passenger,Airlift,150);passenger->mTargetCol=int(passenger->mPosX);passenger->mTargetPlantID=static_cast<decltype(passenger->mTargetPlantID)>(b->mZombies.DataArrayGetID(z));z->mSummonCounter=1;gLawnApp->PlayMemeCue(3,-3);}
  }
 }
}
void DrawPortrait(Sexy::Graphics*,int,int,int,int,int){}
float Speed(Zombie* z){
 if(!Enabled()||!Walker(z))return 1.0f;
 if(IsRetreating(z))return -1.4f;
 if(IsFeigning(z)||IsResting(z))return 0.0f;
 return 1.0f; // Native pace; flag encouragement and football skidding are physical actions.
}
int Damage(Zombie*,int damage,unsigned){return damage;} // A door must physically intercept the shot.
bool ElectricHit(Zombie*){return false;} void CombatDeath(Zombie*){}
void DrawEffects(Sexy::Graphics* g,Board* b,int row){
 // Draw the tow-line in lawn coordinates. Passenger and carrier retain their
 // own native bodies, armor, hit reactions and shadows.
 for(auto* z:b->mZombies)if(!z->mDead&&int(z->mZombiePhase)==Airlift&&z->mRow==row){
  auto* carrier=b->mZombies.DataArrayTryToGet(static_cast<unsigned>(z->mTargetPlantID));
  if(!carrier||carrier->mDead)continue;
  const int x1=int(carrier->mPosX+72),y1=int(carrier->mPosY+66-carrier->mAltitude),x2=int(z->mPosX+44),y2=int(z->mPosY+53-z->mAltitude);
  g->SetColor(Sexy::Color(68,61,37));g->DrawLine(x1,y1,x2,y2);g->DrawLine(x1,y1+1,x2,y2+1);
 }
 // Native thorn art across the two ankles: the binding is visible, with no labels.
 auto* thorn=SandboxArt::NativeImage("SpikeRock_spike.png");if(!thorn)return;
 for(auto* z:b->mZombies)if(!z->mDead&&int(z->mZombiePhase)==Pinned&&z->mRow==row){
  const float y=z->mPosY+112,x=z->mPosX+45;
  Sexy::SexyTransform2D m;m.LoadIdentity();m.m00=.4f;m.m11=.4f;m.m02=x+g->mTransX;m.m12=y+g->mTransY;
  PvzpBltMatrix(g,thorn,m,g->mClipRect,Sexy::Color(255,255,255),g->mDrawMode,Sexy::Rect(0,0,thorn->mWidth,thorn->mHeight));
  for(auto* other:b->mZombies)if(other!=z&&!other->mDead&&int(other->mZombiePhase)==Pinned&&other->mTargetCol==z->mTargetCol&&other->mRow==row&&other->mPosX>z->mPosX){
   g->SetColor(Sexy::Color(65,70,29));g->DrawLine(int(x),int(y),int(other->mPosX+45),int(other->mPosY+112));
   g->SetColor(Sexy::Color(179,177,102));g->DrawLine(int(x),int(y-1),int(other->mPosX+45),int(other->mPosY+111));
  }
 }
}
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
