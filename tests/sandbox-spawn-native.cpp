// Compile the real sandbox Spawn function with state-only native doubles.
#include "SandboxRules.h"
#include <cassert>
#include <iostream>
enum ZombieType {ZOMBIE_NORMAL=0,ZOMBIE_DUCKY_TUBE=10,ZOMBIE_SNORKEL=11,ZOMBIE_DOLPHIN_RIDER=14,ZOMBIE_BALLOON=16};
struct Zombie {
 static constexpr int ZOMBIE_WAVE_DEBUG=-1;
 float mPosX=0;int mX=0;bool updated=false;
 void UpdateReanim(){updated=true;}
 static void PreloadZombieResources(ZombieType){}
 static bool ZombieTypeCanGoInPool(ZombieType type){return type==0||type==1||type==2||type==4||type==11||type==14;}
};
int mapType=1;
struct Board {
 struct {int mSize=0,mMaxSize=1024;} mZombies;
 int count=0,type=-1,row=-1;bool marked=false,allocationFails=false;Zombie zombie;
 bool IsPoolSquare(int,int r){return mapType==1&&(r==2||r==3);}
 int GridToPixelX(int col,int){return 40+col*80;}
 Zombie* AddZombieInRow(ZombieType t,int r,int){if(allocationFails)return nullptr;type=t;row=r;++count;++mZombies.mSize;return &zombie;}
 void MarkAllDirty(){marked=true;}
};
int assignedZombie=-1;
namespace SandboxZombies {void Assign(Zombie*,int id){assignedZombie=id;}}
int ZombieCount(Board* b){return b->count;}
#include "spawn-under-test.inc"
int main(){
 Board b;
 for(int id:{212,213})for(int row:{0,2,3,4}){Board custom;assert(Spawn(&custom,id,8,row)==1&&custom.type==ZOMBIE_NORMAL&&assignedZombie==id);}
 for(int row:{0,2,3,4}){Board custom;assert(Spawn(&custom,214,8,row)==1&&custom.type==2&&assignedZombie==214);}
 assert(Spawn(&b,10,8,2)==1&&b.type==ZOMBIE_NORMAL&&b.row==2);
 assert(b.zombie.mPosX==690&&b.zombie.mX==690&&b.zombie.updated&&b.marked);
 for(int waterOnly:{10,11,14}){Board land;assert(Spawn(&land,waterOnly,8,0)==-5&&land.count==0);}
 for(int swimmer:{0,1,2,4,10,11,14,16}){Board water;assert(Spawn(&water,swimmer,8,3)==1);assert(water.type==(swimmer==10?0:swimmer));}
 for(int landOnly:{3,7,12,23,32}){Board water;assert(Spawn(&water,landOnly,8,2)==-5&&water.count==0);}
 for(int invalid:{-1,9,13,20,200}){Board water;assert(Spawn(&water,invalid,8,2)==-2);}
 for(auto cell:{std::pair{-1,2},std::pair{9,2},std::pair{1,-1},std::pair{1,6}}){Board water;assert(Spawn(&water,10,cell.first,cell.second)==-2);}
 Board full;full.count=SandboxRules::MaxZombies;assert(Spawn(&full,10,8,2)==-3);
 Board poolFull;poolFull.mZombies.mSize=poolFull.mZombies.mMaxSize-8;assert(Spawn(&poolFull,10,8,2)==-3);
 Board noMemory;noMemory.allocationFails=true;assert(Spawn(&noMemory,10,8,2)==-3);
 mapType=0;Board day;assert(Spawn(&day,10,8,2)==-5&&Spawn(&day,0,8,2)==1);
 std::cout<<"Production zombie spawning and native duck normalization passed\n";
}
