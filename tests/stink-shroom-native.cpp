#include "StinkShroom.h"
#include "StinkShroomRules.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <vector>
enum {ZOMBIE_NORMAL=0,ZOMBIE_GARGANTUAR=23,ZOMBIE_REDEYE_GARGANTUAR=32,ZOMBIE_BOSS=25,ZOMBIE_BUNGEE=20,HEIGHT_ZOMBIE_NORMAL=0,PHASE_ZOMBIE_NORMAL=0,FOLEY_YUCK=0};
namespace Sexy {int roll=99;int Rand(int n){assert(n==100);return roll;}}
class Board;class Zombie;
struct App {int sounds=0;void PlayFoley(int){++sounds;}};
struct Plant {Board* mBoard;App* mApp;int type=0,mPlantCol=2,mRow=2,mPlantHealth=300;bool mDead=false,mSquished=false,mIsAsleep=false,air=false;bool NotOnGround(){return air;}};
namespace MemeCharacters {int Type(const Plant* p){return p->type;}}
struct Zombie {Board* mBoard;bool mDead=false,dying=false,onBoard=true,mMindControlled=false,mInPool=false,chilled=false,eating=false;
 int mZombieType=ZOMBIE_NORMAL,mZombieHeight=0,mZombiePhase=0,mPhaseCounter=0,mTargetRow=2,mRow=2,mX=400,mY=170,mIceTrapCounter=0,mButteredCounter=0,animUpdates=0;
 float mAltitude=0,mPosX=400,mPosY=170,animRate=12;
 bool IsOnBoard(){return onBoard;}bool IsDeadOrDying(){return dying;}void StopEating(){eating=false;}void StartWalkAnim(int){animRate=12;}void UpdateAnimSpeed(){++animUpdates;animRate=StinkShroom::Stunned(this)?0:12;}
 bool IsMovingAtChilledSpeed(){return chilled;}void ApplyAnimRate(float rate){animRate=rate;}
 float GetPosYBasedOnRow(int row){return row*85.f;}
 void CheckForBoardEdge(){if(mPosX>850){mDead=true;StinkShroom::Forget(this);}}
};
template<class T>struct Array:std::vector<T*>{using std::vector<T*>::operator=;unsigned DataArrayGetID(T* p){for(unsigned i=0;i<this->size();++i)if(this->at(i)==p)return i+1;return 0;}T* DataArrayTryToGet(unsigned key){return key&&key<=this->size()?this->at(key-1):nullptr;}};
class Board {public:App app;bool mPaused=false;unsigned mMainCounter=0;Array<Plant> mPlants;Array<Zombie> mZombies;};
#include "stink-production.inc"
void near(float a,float b){assert(std::abs(a-b)<.005f);}
int main(){
 using namespace StinkShroomRules;
 static_assert(Interval==200&&Cost==75&&Damage==20&&Recharge==750);
 {Board b;Plant source{&b,&b.app},nearby{&b,&b.app},far{&b,&b.app},same{&b,&b.app};source.type=527;nearby.mPlantCol=3;far.mPlantCol=4;
  b.mPlants={&source,&nearby,&far,&same};StinkShroom::Reset();
  for(int tick=1;tick<=1999;++tick){++b.mMainCounter;for(auto* p:b.mPlants)StinkShroom::UpdatePlant(p);assert(!StinkShroom::Affected(&nearby));}
  b.mPaused=true;StinkShroom::UpdatePlant(&nearby);assert(StinkShroom::Capture(&b).plants[0].exposure==1999);b.mPaused=false;
  ++b.mMainCounter;for(auto* p:b.mPlants)StinkShroom::UpdatePlant(p);
  assert(StinkShroom::Affected(&nearby)&&!StinkShroom::Affected(&source)&&!StinkShroom::Affected(&far)&&!StinkShroom::Affected(&same));
  int work=0;for(int tick=0;tick<1000;++tick){++b.mMainCounter;StinkShroom::UpdatePlant(&nearby);work+=StinkShroom::WorkTick(&nearby);}assert(work==700);
  const auto saved=StinkShroom::Capture(&b);StinkShroom::Reset();StinkShroom::Load(saved);StinkShroom::Restore(&b);assert(StinkShroom::Affected(&nearby));
  // Removal restores immediately, even before the next plant update.
  source.mDead=true;assert(!StinkShroom::Affected(&nearby)&&StinkShroom::WorkTick(&nearby));StinkShroom::UpdatePlant(&nearby);assert(StinkShroom::Capture(&b).plants.empty());
  source.mDead=false;source.mIsAsleep=true;for(int i=0;i<2000;++i)StinkShroom::UpdatePlant(&nearby);assert(!StinkShroom::Affected(&nearby));
 }
 // All eight neighbours, never two tiles away; two sources cannot stack the penalty.
 for(int dc=-2;dc<=2;++dc)for(int dr=-2;dr<=2;++dr){Board b;Plant a{&b,&b.app},p{&b,&b.app},a2{&b,&b.app};a.type=a2.type=527;p.mPlantCol+=dc;p.mRow+=dr;b.mPlants={&a,&a2,&p};StinkShroom::Reset();
  for(int i=0;i<2000;++i)StinkShroom::UpdatePlant(&p);assert(StinkShroom::Affected(&p)==Adjacent(p.mPlantCol,p.mRow,2,2));
 }
 // Every random value traverses the actual production hit branch.
 {Board b;Plant a{&b,&b.app},a2{&b,&b.app},neighbour{&b,&b.app};a.type=a2.type=527;a2.mPlantCol=3;neighbour.mRow=1;
  b.mPlants={&a,&a2,&neighbour};StinkShroom::Reset();
  for(int i=0;i<2500;++i)for(auto* p:b.mPlants)StinkShroom::UpdatePlant(p);
  assert(!StinkShroom::Affected(&a)&&!StinkShroom::Affected(&a2)&&StinkShroom::Affected(&neighbour));
  for(int tick=0;tick<100;++tick){b.mMainCounter=tick;assert(StinkShroom::WorkTick(&a)&&StinkShroom::WorkTick(&a2));}
  // Old saves may contain exposure on another Fume-shroom: it is immune too.
  StinkShroom::Load({{{1,2000},{2,2000}}, {}});StinkShroom::Restore(&b);
  assert(!StinkShroom::Affected(&a)&&!StinkShroom::Affected(&a2));
 }
 int counts[4]{};
 for(int roll=0;roll<100;++roll){Board b;Plant p{&b,&b.app};p.type=527;Zombie z{&b};b.mZombies={&z};StinkShroom::Reset();Sexy::roll=roll;StinkShroom::Hit(&p,&z);
  const auto result=Roll(roll);++counts[result];const auto save=StinkShroom::Capture(&b);
  if(result==Normal){assert(save.zombies.empty());continue;}
  assert(save.zombies.size()==1&&StinkShroom::Controls(&z));const auto record=save.zombies[0];
  if(result==Stun){assert(record.stun==50&&z.animRate==0);for(int i=0;i<50;++i){assert(StinkShroom::Stunned(&z));assert(StinkShroom::UpdateZombie(&z));near(z.mPosX,400);}assert(!StinkShroom::Stunned(&z)&&z.animRate==12);assert(!StinkShroom::UpdateZombie(&z));}
  if(result==Push){assert(record.push==20&&record.stepMilli==2000);for(int i=0;i<20;++i)assert(StinkShroom::UpdateZombie(&z));near(z.mPosX,440);assert(!StinkShroom::UpdateZombie(&z));}
  if(result==Flee){assert(record.flee==1&&StinkShroom::Fleeing(&z));for(int i=0;i<10;++i)StinkShroom::UpdateZombie(&z);near(z.mPosX,432);z.mIceTrapCounter=1;StinkShroom::UpdateZombie(&z);near(z.mPosX,432);z.mIceTrapCounter=0;
   while(!z.mDead)StinkShroom::UpdateZombie(&z);assert(z.mPosX>850&&!StinkShroom::Controls(&z));}
 }
 assert(counts[Normal]==35&&counts[Stun]==50&&counts[Flee]==5&&counts[Push]==10);
 for(int type:{ZOMBIE_GARGANTUAR,ZOMBIE_REDEYE_GARGANTUAR}){Board b;Plant p{&b,&b.app};p.type=527;Zombie z{&b};z.mZombieType=type;b.mZombies={&z};StinkShroom::Reset();Sexy::roll=55;StinkShroom::Hit(&p,&z);for(int i=0;i<20;++i)StinkShroom::UpdateZombie(&z);near(z.mPosX,416);}
 // Status save/resume retains exact remaining time/distance and does not heal.
 for(int roll:{0,50,55}){Board b;Plant p{&b,&b.app};p.type=527;Zombie z{&b};b.mZombies={&z};StinkShroom::Reset();Sexy::roll=roll;StinkShroom::Hit(&p,&z);for(int i=0;i<7;++i)StinkShroom::UpdateZombie(&z);
  const auto save=StinkShroom::Capture(&b);const float x=z.mPosX;StinkShroom::Reset();StinkShroom::Load(save);StinkShroom::Restore(&b);const auto again=StinkShroom::Capture(&b);near(z.mPosX,x);
  assert(again.zombies.size()==1&&again.zombies[0].stun==save.zombies[0].stun&&again.zombies[0].push==save.zombies[0].push&&again.zombies[0].flee==save.zombies[0].flee);
  b.mPaused=true;StinkShroom::UpdateZombie(&z);near(z.mPosX,x);assert(StinkShroom::Capture(&b).zombies[0].stun==save.zombies[0].stun);
 }
 for(int guard=0;guard<7;++guard){Board b;Plant p{&b,&b.app};p.type=527;Zombie z{&b};b.mZombies={&z};StinkShroom::Reset();Sexy::roll=50;
  if(guard==0)z.mDead=true;if(guard==1)z.onBoard=false;if(guard==2)z.dying=true;if(guard==3)z.mMindControlled=true;if(guard==4)z.mZombieType=ZOMBIE_BOSS;if(guard==5)z.mZombieType=ZOMBIE_BUNGEE;if(guard==6)z.mZombieHeight=1;
  StinkShroom::Hit(&p,&z);assert(!StinkShroom::Controls(&z));
 }
 std::cout<<"Stink shroom: exact 50/5/10 rolls, 0.5-second stun, smooth push/flee, 20-second exposure, 70-percent work and saved statuses passed\n";
}
