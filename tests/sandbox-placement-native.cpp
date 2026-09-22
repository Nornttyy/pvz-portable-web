#include "SandboxRules.h"
#include "SandboxUIRules.h"
#include <cassert>
#include <vector>
#include <memory>
#include <iostream>
enum SeedType { SEED_NONE=-1, SEED_PEASHOOTER=0, SEED_GRAVEBUSTER=11, SEED_LILYPAD=16, SEED_FLOWERPOT=33, SEED_INSTANT_COFFEE=35, SEED_CATTAIL=43, SEED_COBCANNON=47 };
enum { PLANTING_OK, PLANTING_BLOCKED };
class Plant {
public:
 int mPlantCol=0,mRow=0,id=0;SeedType mSeedType=SEED_PEASHOOTER;
 bool mDead=false,mIsAsleep=false;
 void Die(){mDead=true;} void SetSleeping(bool v){mIsAsleep=v;}
 bool NotOnGround(){return false;}
 bool IsUpgradableTo(SeedType seed){return seed==40&&mSeedType==7;}
 static bool IsUpgrade(SeedType seed){return seed>=40;}
 static void PreloadPlantResources(SeedType){}
};
namespace SandboxPlants { void Assign(Plant* p,int id){p->id=id;} }
struct PlantsOnLawn {Plant* mUnderPlant=nullptr;Plant* mNormalPlant=nullptr;};
struct Plants : std::vector<Plant*> {int mSize=0,mMaxSize=256;};
class Board {
public:
 Plants mPlants;std::vector<std::unique_ptr<Plant>> owned;
 bool pool=false,blocked=false;int nativeChecks=0,marks=0;
 bool IsPoolSquare(int,int row){return pool&&(row==2||row==3);}
 bool GetGraveStoneAt(int,int){return blocked;}
 bool GetCraterAt(int,int){return false;}
 bool GetScaryPotAt(int,int){return false;}
 bool IsIceAt(int,int){return false;}
 int CanPlantAt(int col,int row,SeedType seed){
   ++nativeChecks;if(blocked)return PLANTING_BLOCKED;
   if(seed==SEED_GRAVEBUSTER||seed==SEED_INSTANT_COFFEE)return PLANTING_BLOCKED;
   for(auto* p:mPlants)if(!p->mDead&&p->mPlantCol==col&&p->mRow==row&&!p->IsUpgradableTo(seed))return PLANTING_BLOCKED;
   return PLANTING_OK;
 }
 void GetPlantsOnLawn(int col,int row,PlantsOnLawn* out){
   for(auto* p:mPlants)if(!p->mDead&&p->mPlantCol==col&&p->mRow==row){
     if(p->mSeedType==SEED_LILYPAD||p->mSeedType==SEED_FLOWERPOT)out->mUnderPlant=p;else out->mNormalPlant=p;
   }
 }
 Plant* AddPlant(int col,int row,SeedType seed,SeedType){
   auto p=std::make_unique<Plant>();p->mPlantCol=col;p->mRow=row;p->mSeedType=seed;
   auto* result=p.get();owned.push_back(std::move(p));mPlants.push_back(result);++mPlants.mSize;return result;
 }
 void MarkAllDirty(){++marks;}
};
bool stackPlants=false,awake=true;int mapType=0;
int PlantCount(Board* b){int n=0;for(auto* p:b->mPlants)if(!p->mDead)++n;return n;}
// Inserted verbatim from production Sandbox.cpp by the test runner.
#include "placement-under-test.inc"
int main(){
 using SandboxUIRules::RepeatPlacement;
 RepeatPlacement repeat;
 assert(!repeat.Poll(0,0,true,true));repeat.Begin(0,0);
 for(int ms=0;ms<300;++ms)assert(!repeat.Poll(0,ms,true,true));
 assert(repeat.Poll(0,300,true,true));assert(!repeat.Poll(0,300,true,true));
 assert(repeat.Poll(1,301,true,true));assert(!repeat.Poll(-1,302,true,true));
 assert(repeat.Poll(1,303,true,true));assert(!repeat.Poll(1,1000,false,true));
 assert(!repeat.Poll(1,2000,true,true)); // no restart without a fresh press
 repeat.Begin(2,0);assert(!repeat.Poll(2,500,true,false));assert(!repeat.active);
 repeat.Begin(2,0);assert(repeat.Poll(2,9000,true,true));assert(!repeat.Poll(2,9000,true,true)); // no catch-up burst
 repeat.Stop();assert(!repeat.active);
 Board b;
 assert(PlacePlant(&b,0,2,2)==1);assert(PlacePlant(&b,0,2,2)==-4);
 stackPlants=true;
 assert(PlacePlant(&b,0,2,2)==1);assert(PlacePlant(&b,105,2,2)==1);
 assert(PlantCount(&b)==3&&b.mPlants.back()->id==105); // upgrade leaves both originals alive
 stackPlants=false;assert(PlacePlant(&b,7,2,2)==-4);assert(PlantCount(&b)==3);
 Board normal;assert(PlacePlant(&normal,7,0,0)==1);assert(PlacePlant(&normal,40,0,0)==1);
 assert(PlantCount(&normal)==1&&normal.mPlants.front()->mDead);
 stackPlants=true;
 assert(PlacePlant(&b,47,8,0)==-4);assert(PlacePlant(&b,47,7,0)==1);assert(PlacePlant(&b,0,8,0)==1);
 for(int seed:{11,16,19,24,35,43})assert(PlacePlant(&b,seed,0,0)==-4);
 b.blocked=true;assert(PlacePlant(&b,0,0,0)==-4);b.blocked=false;
 mapType=1;Board pool;pool.pool=true;
 assert(PlacePlant(&pool,0,1,2)==-4);assert(PlacePlant(&pool,16,1,2)==1);
 assert(PlacePlant(&pool,0,1,2)==1);assert(PlacePlant(&pool,0,1,2)==1);
 for(int seed:{4,21,33,46})assert(PlacePlant(&pool,seed,1,2)==-4);
 assert(PlacePlant(&pool,47,1,2)==-4);assert(PlacePlant(&pool,16,2,2)==1);
 assert(PlacePlant(&pool,47,1,2)==1);assert(PlacePlant(&pool,43,1,2)==1);
 mapType=0;Board cap;
 for(int i=0;i<180;++i)assert(PlacePlant(&cap,0,0,0)==1);
 assert(PlacePlant(&cap,0,0,0)==-3);assert(PlantCount(&cap)==180);
 Board slots;slots.mPlants.mSize=248;assert(PlacePlant(&slots,0,0,0)==-3);
 assert(PlacePlant(&b,999,0,0)==-2);assert(PlacePlant(&b,0,-1,0)==-2);
 std::cout<<"Production placement, stack terrain, capacity and continuous input passed\n";
}
