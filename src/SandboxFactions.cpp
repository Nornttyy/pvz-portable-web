// Sandbox-only allegiances. Native adventure objects and save format are unchanged.
#include "SandboxFactions.h"
#include "Sandbox.h"
#include "SandboxPlants.h"
#include "MemeCharacters.h"
#include "EverythingShooterRules.h"
#include "CoinPlantRules.h"
#include "LawnApp.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "Lawn/Projectile.h"
#include "PvzpLib/PvzpCommon.h"
#include "PvzpLib/Reanimator.h"
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <array>
namespace SandboxFactions {
namespace {
std::set<const Plant*> plants;
std::set<const GridItem*> craters;
std::map<const Plant*,std::array<unsigned,CoinPlantRules::Limit>> orbitContacts;
std::map<const Plant*,int> frozen,melee;
struct Shot {bool hostile;unsigned target;};
std::map<const Projectile*,Shot> shots;
Plant* source=nullptr;
Zombie* zombieSource=nullptr;
const Plant* drawing=nullptr;
bool Live(Plant* p){return p&&!p->mDead&&p->mPlantHealth>0&&!p->NotOnGround();}
int ShotDamage(Projectile* s){
 const int style=MemeCharacters::ShotStyle(s);
 if(CoinPlantRules::Shot(style))return CoinPlantRules::Damage(CoinPlantRules::ShotKind(style));
 if(EverythingShooterRules::Special(style))return style==EverythingShooterRules::Poop?EverythingShooterRules::PoopDamage:EverythingShooterRules::BlastDamage(style);
 if(style==EverythingShooterRules::Cob)return EverythingShooterRules::CobDamage;
 return SandboxPlants::ShotDamage(s,s->GetProjectileDef().mDamage);
}
}
void Reset(){plants.clear();craters.clear();orbitContacts.clear();shots.clear();frozen.clear();melee.clear();source=nullptr;zombieSource=nullptr;drawing=nullptr;}
bool Charmed(const Plant* p){return gSandboxEnabled&&p&&plants.contains(p);}
bool Charmed(const GridItem* p){return gSandboxEnabled&&craters.contains(p);}
void Set(GridItem* p,bool on){if(on&&gSandboxEnabled)craters.insert(p);else craters.erase(p);}
void Set(Plant* p,bool on){if(!gSandboxEnabled||!p)return;if(on)plants.insert(p);else plants.erase(p);p->UpdateReanimColor();}
void Set(Zombie* z,bool on){
 if(!gSandboxEnabled||!z)return;
 // Preserve native linked teams: StartMindControlled detaches bobsled leaders.
 z->mMindControlled=on;z->mLastPortalX=-1;z->StopEating();z->UpdateReanim();
 for(auto id:z->mFollowerZombieID)if(auto* follower=z->mBoard->ZombieTryToGet(id)){follower->mMindControlled=on;follower->UpdateReanim();}
}
void Forget(const Plant* p){plants.erase(p);frozen.erase(p);melee.erase(p);orbitContacts.erase(p);}
void Forget(const Projectile* p){shots.erase(p);}
bool Enemy(const Plant* p,const Plant* other){return p!=other&&Charmed(p)!=Charmed(other);}
bool Enemy(const Plant* p,const Zombie* z){return z&&(!gSandboxEnabled?!z->mMindControlled:Charmed(p)==z->mMindControlled);}
bool Enemy(const Zombie* z,const Plant* p){return !gSandboxEnabled||z->mMindControlled==Charmed(p);}
bool NutContact(Plant* p,Zombie* z,int padding){return Charmed(p)?z->FindPlantTarget(ATTACKTYPE_CHEW)==p:z->mPosX>=p->mX-65&&z->mPosX<p->mX+padding;}
int Flags(const Plant* p,int flags){return Charmed(p)?flags|128:flags;}
Sexy::Rect AttackRect(const Plant* p,Sexy::Rect r){if(Charmed(p)&&p->mSeedType!=SEED_CATTAIL)r.mX=2*p->mX+80-r.mX-r.mWidth;return r;}
Plant* Target(Plant* p,int row,int weapon){
 if(!gSandboxEnabled||!p)return nullptr;
 const auto rect=p->GetPlantAttackRect(static_cast<PlantWeapon>(weapon));Plant* best=nullptr;int nearest=100000;
 for(auto* q:p->mBoard->mPlants){
  if(!Live(q)||!Enemy(p,q)||(q->mRow!=row&&p->mSeedType!=SEED_CATTAIL&&p->mSeedType!=SEED_GLOOMSHROOM))continue;
  if(p->mSeedType==SEED_GLOOMSHROOM&&std::abs(q->mRow-row)>1)continue;
  if(GetRectOverlap(rect,q->GetPlantRect())<0)continue;
  const int distance=std::abs(q->mX-p->mX)+std::abs(q->mY-p->mY);
  if(distance<nearest||(distance==nearest&&q->mSeedType==SEED_PUMPKINSHELL)){best=q;nearest=distance;}
 }
 return best;
}
bool HasTarget(Plant* p,int row,int weapon){return p->FindTargetZombie(row,static_cast<PlantWeapon>(weapon))||Target(p,row,weapon);}
void OnFired(Plant* p,Projectile* s,Zombie* z){
 if(!gSandboxEnabled||!p||!s)return;
 const bool hostile=Charmed(p);auto* target=z?nullptr:Target(p,s->mRow);
 shots[s]={hostile,target?p->mBoard->mPlants.DataArrayGetID(target):0};
 s->mDamageRangeFlags=Flags(p,s->mDamageRangeFlags);
 if(hostile){s->mPosX=2*p->mX+80-(s->mPosX+s->mWidth);s->mX=int(s->mPosX);if(s->mMotionType==MOTION_STAR)s->mVelX=-s->mVelX;}
 if(s->mMotionType==MOTION_LOBBED&&(hostile||target)){
  const float tx=target?target->mX+30:z?z->ZombieTargetLeadX(50)-30:0;
  const float ty=target?target->mY:z?z->GetZombieRect().mY:p->mY;
  if(s->mProjectileType==PROJECTILE_COBBIG){s->mCobTargetX=std::clamp(tx-40,0.f,720.f);s->mCobTargetRow=target?target->mRow:p->mRow;}
  else{s->mVelX=(tx-s->mPosX)/120.f;s->mVelZ=(ty-s->mPosY)/120.f-7.f;}
 }
 if(s->mMotionType==MOTION_HOMING&&target){s->mTargetZombieID=ZOMBIEID_NULL;s->mVelX=(target->mX+40-s->mPosX)/120.f;}
}
int Direction(const Projectile* s){const auto it=shots.find(s);return gSandboxEnabled&&it!=shots.end()&&it->second.hostile?-1:1;}
void Home(Projectile* s){
 if(!gSandboxEnabled||s->mMotionType!=MOTION_HOMING)return;
 auto it=shots.find(s);if(it==shots.end()||!it->second.target)return;
 auto* p=s->mBoard->mPlants.DataArrayTryToGet(it->second.target);if(!Live(p)||!ShotEnemy(s,p))return;
 const auto r=p->GetPlantRect();const float dx=r.mX+r.mWidth*.5f-s->mPosX-s->mWidth*.5f,dy=r.mY+r.mHeight*.5f-s->mPosY-s->mHeight*.5f;
 const float length=std::max(1.f,std::hypot(dx,dy));s->mVelX=dx/length*2;s->mVelY=dy/length*2;s->mRotation=-std::atan2(s->mVelY,s->mVelX);
}
bool ShotEnemy(const Projectile* s,const Plant* p){
 if(!gSandboxEnabled)return true;
 const auto it=shots.find(s);
 const bool hostile=it!=shots.end()?it->second.hostile:s->mProjectileType==PROJECTILE_ZOMBIE_PEA||s->mProjectileType==PROJECTILE_BASKETBALL;
 return hostile!=Charmed(p);
}
void Damage(Plant* p,int amount,int freeze){
 if(!Live(p))return;
 p->mPlantHealth-=amount;p->mEatenFlashCountdown=25;
 if(freeze>0)frozen[p]=std::max(frozen[p],freeze);
 if(p->mPlantHealth<=0)p->Die();
}
bool OrbitHit(Plant* p,int slot,float x,float y,int damage){
 if(!gSandboxEnabled||slot<0||slot>=CoinPlantRules::Limit)return false;
 unsigned contact=0;bool hit=false;auto& last=orbitContacts[p][slot];
 for(auto* target:p->mBoard->mPlants){
  if(!Live(target)||!Enemy(p,target)||std::abs(target->mRow-p->mRow)>1||std::abs(target->mPlantCol-p->mPlantCol)>1)continue;
  const auto r=target->GetPlantRect();if(x+9<r.mX||x-9>r.mX+r.mWidth||y+9<r.mY||y-9>r.mY+r.mHeight)continue;
  contact=p->mBoard->mPlants.DataArrayGetID(target);if(last!=contact){Damage(target,damage);hit=true;}break;
 }
 last=contact;return hit;
}
bool HitPlant(Projectile* s,bool lob){
 if(!gSandboxEnabled||s->mDead||!MemeCharacters::CanHit(s))return false;
 // Native zombie peas use their own plant collision and can never hit allies.
 auto it=shots.find(s);if(it==shots.end()&&s->mProjectileType!=PROJECTILE_PEA)return false;
 if(lob&&(s->mVelZ<0||s->mPosZ<-32))return false;
 Plant* target=nullptr;float distance=1e9f;auto r=s->GetProjectileRect();
 for(auto* p:s->mBoard->mPlants){
  if(!Live(p)||!ShotEnemy(s,p)||p->mRow!=s->mRow||!MemeCharacters::CanHitRow(s,p->mRow))continue;
  const auto pr=p->GetPlantRect();
  if(GetRectOverlap(r,pr)<0||r.mY>pr.mY+pr.mHeight||r.mY+r.mHeight<pr.mY)continue;
  float d=std::abs(pr.mX-r.mX);
  if(d<distance||(d==distance&&p->mSeedType==SEED_PUMPKINSHELL)){target=p;distance=d;}
 }
 if(!target)return false;
 const int damage=ShotDamage(s),style=MemeCharacters::ShotStyle(s);
 const bool splash=!EverythingShooterRules::Own(style)&&!CoinPlantRules::Shot(style)&&s->IsSplashDamage(nullptr);
 const int freeze=s->mProjectileType==PROJECTILE_BUTTER?300:s->mProjectileType==PROJECTILE_SNOWPEA||s->mProjectileType==PROJECTILE_WINTERMELON?100:0;
 if(splash){for(auto* p:s->mBoard->mPlants)if(Live(p)&&ShotEnemy(s,p)&&std::abs(p->mRow-target->mRow)<=1&&std::abs(p->mX-target->mX)<100)Damage(p,p==target?damage:damage/3,freeze);}
 else Damage(target,damage,freeze);
 s->DoImpact(nullptr);if(!s->mDead)s->Die();return true;
}
void Area(Plant* p,int x,int y,int radius,int rows,int damage,int freeze){
 if(!gSandboxEnabled||!p)return;
 for(auto* q:p->mBoard->mPlants)if(Live(q)&&Enemy(p,q)&&std::abs(q->mRow-p->mRow)<=rows&&GetCircleRectOverlap(x,y,radius,q->GetPlantRect()))Damage(q,damage,freeze);
}
void RowDamage(Plant* p,int row,int damage,int freeze){if(gSandboxEnabled)for(auto* q:p->mBoard->mPlants)if(Live(q)&&q->mRow==row&&Enemy(p,q))Damage(q,damage,freeze);}
bool UpdatePlant(Plant* p){
 if(!gSandboxEnabled)return false;
 if(auto it=frozen.find(p);it!=frozen.end()){if(--it->second<=0)frozen.erase(it);return true;}
 // Native melee implementations address zombies. Keep their native attack
 // interval/animation, but resolve a plant opponent without fake zombie actors.
 if(p->mIsAsleep||p->mSquished||p->NotOnGround())return false;
 const int type=p->mSeedType;
 if(type!=SEED_CHOMPER&&type!=SEED_SQUASH&&type!=SEED_TANGLEKELP&&type!=SEED_POTATOMINE&&type!=SEED_SPIKEWEED&&type!=SEED_SPIKEROCK)return false;
 auto* target=Target(p,p->mRow);if(!target)return false;
 if(p->FindTargetZombie(p->mRow))return false;
 auto& time=melee[p];if(time>0){--time;return true;}
 if(type==SEED_POTATOMINE&&p->mState!=STATE_POTATO_ARMED)return false;
 if(type==SEED_SPIKEWEED||type==SEED_SPIKEROCK){Damage(target,type==SEED_SPIKEROCK?40:20);time=100;return true;}
 if(type==SEED_CHOMPER){Damage(target,1800);time=4200;p->PlayBodyReanim("anim_bite",REANIM_PLAY_ONCE_AND_HOLD,10,20);}
 else{Area(p,target->mX+40,target->mY+40,75,0,1800);p->mApp->AddPvzpParticle(target->mX+40,target->mY+40,RENDER_LAYER_TOP,PARTICLE_POWIE);p->Die();}
 return true;
}
bool Frozen(const Plant* p){return gSandboxEnabled&&frozen.contains(p);}
ZombieScope::ZombieScope(Zombie* z):previous(zombieSource){zombieSource=gSandboxEnabled?z:nullptr;}
ZombieScope::~ZombieScope(){zombieSource=previous;}
Zombie* ZombieSource(){return zombieSource;}
void SmashZombies(Zombie* source,int col,int row,int width,int height){
 if(!gSandboxEnabled)return;
 for(auto* z:source->mBoard->mZombies){
  if(z==source||z->mDead||z->IsDeadOrDying()||!z->IsOnBoard()||z->mMindControlled==source->mMindControlled||z->mRow<row||z->mRow>=row+height)continue;
  const int c=source->mBoard->PixelToGridXKeepOnBoard(z->GetZombieRect().mX+30,z->mPosY);
  if(c>=col&&c<col+width)z->TakeDamage(1800,18U);
 }
}
Scope::Scope(Plant* p):previous(source){source=gSandboxEnabled?p:nullptr;}
Scope::~Scope(){source=previous;}
Plant* Source(){return source;}
DrawScope::DrawScope(const Plant* p):previous(drawing){drawing=Charmed(p)?p:nullptr;}
DrawScope::~DrawScope(){drawing=previous;}
void DrawMatrix(Sexy::SexyMatrix3& m,Sexy::Color* colour){
 if(!drawing)return;
 m.m00=-m.m00;m.m01=-m.m01;m.m02=80-m.m02;
 if(colour){colour->mRed=colour->mRed*160/255;colour->mGreen=colour->mGreen*105/255;}
}
void DrawOverlay(Sexy::SexyMatrix3& m,float x,float y){m.m02-=x;m.m12-=y;DrawMatrix(m);m.m02+=x;m.m12+=y;}
}
