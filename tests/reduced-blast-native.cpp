#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
enum ZombieType {ZOMBIE_NORMAL,ZOMBIE_BOSS};
enum GridItemType {GRIDITEM_LADDER,GRIDITEM_OTHER};
struct Rect {int x,y,w,h;};
bool GetCircleRectOverlap(int x,int y,int radius,Rect r){int dx=x-std::clamp(x,r.x,r.x+r.w),dy=y-std::clamp(y,r.y,r.y+r.h);return dx*dx+dy*dy<=radius*radius;}
bool GridInRange(int x,int y,int gx,int gy,int dx,int dy){return std::abs(x-gx)<=dx&&std::abs(y-gy)<=dy;}
struct Zombie {
 bool mDead=false,eligible=true;int mRow=2,mBodyHealth=6000,mHelmHealth=0,mShieldHealth=0,hits=0,burns=0,damage=0;ZombieType mZombieType=ZOMBIE_NORMAL;Rect rect{390,220,60,80};
 bool EffectedByDamage(int f){assert(f==127);return eligible;}Rect GetZombieRect(){return rect;}
 void ApplyBurn(){++burns;}void TakeDamage(int n,unsigned f){assert(f==18);++hits;damage=n;}
};
struct GridItem{bool mDead=false;GridItemType mGridItemType=GRIDITEM_LADDER;int mGridX=4,mGridY=2;void GridItemDie(){mDead=true;}};
struct Board {
 std::vector<Zombie*> mZombies;std::vector<GridItem*> mGridItems;
 int PixelToGridXKeepOnBoard(int,int){return 4;}int PixelToGridYKeepOnBoard(int,int){return 2;}
 int KillAllZombiesInRadius(int,int,int,int,int,bool,int,int=1800);
};
#include "blast-production.inc"
int main(){
 for(int damage:{400,600}){
  Board b;Zombie giant,normal,bucket,cone,outside,far,dead,friendly,boss;
  normal.mBodyHealth=270;bucket.mBodyHealth=270;bucket.mHelmHealth=1100;cone.mBodyHealth=270;cone.mHelmHealth=370;
  outside.mRow=4;far.rect.x=700;dead.mDead=true;friendly.eligible=false;boss.mZombieType=ZOMBIE_BOSS;boss.mBodyHealth=200;boss.mRow=4;
  b.mZombies={&giant,&normal,&bucket,&cone,&outside,&far,&dead,&friendly,&boss};
  GridItem ladder,out;out.mGridX=8;b.mGridItems={&ladder,&out};
  assert(b.KillAllZombiesInRadius(2,420,250,115,1,true,127,damage)==5);
  for(auto* z:{&giant,&bucket,&cone,&boss})assert(z->hits==1&&z->damage==damage&&z->burns==0);
  assert(normal.burns==1&&!normal.hits&&ladder.mDead&&!out.mDead);
  for(auto* z:{&outside,&far,&dead,&friendly})assert(!z->hits&&!z->burns);
 }
 // All existing callers retain the exact 1800 native burn and damage paths.
 for(bool burn:{false,true}){Board b;Zombie z;b.mZombies={&z};b.KillAllZombiesInRadius(2,420,250,115,1,burn,127);assert(burn?z.burns==1&&z.hits==0:z.damage==1800&&z.hits==1&&z.burns==0);}
 std::cout<<"reduced blasts respect armor, area, boss and native 1800 defaults\n";
}
