#include "SandboxUIRules.h"
#include <cassert>
#include <iostream>
using namespace SandboxUIRules;
bool overlaps(Box a,Box b){return a.x<b.x+b.w&&b.x<a.x+a.w&&a.y<b.y+b.h&&b.y<a.y+a.h;}
int main(){
 static_assert(CanvasWidth==800+WorldOffset&&CanvasHeight==600);
 for(int shake:{0,4,-3,2,-1,0}){assert(BoardX(true,shake)==WorldOffset+shake);assert(BoardX(false,shake)==shake);}
 assert(BoardX(true)==WorldOffset);assert(BoardX(false)==0);
 for(int i=0;i<33;++i){const auto a=SidebarZombie(i);assert(a.x>=0&&a.x+a.w<SidebarWidth&&a.y>=80&&a.y+a.h<=550);for(int j=0;j<i;++j)assert(!overlaps(a,SidebarZombie(j)));}
 for(int i=0;i<25;++i){const auto a=SidebarPlant(i);assert(a.x>=0&&a.x+a.w<=SidebarWidth&&a.y>=80&&a.y+a.h<520);for(int j=0;j<i;++j)assert(!overlaps(a,SidebarPlant(j)));}
 for(int i=0;i<ControlCount;++i){
   const auto b=Control(i);assert(b.x>=548&&b.x+b.w<=CanvasWidth&&b.y+b.h<=80);
   assert(!overlaps(b,Shovel));for(int j=0;j<i;++j)assert(!overlaps(b,Control(j)));
   for(int j=0;j<6;++j)assert(!overlaps(b,Hotbar(j)));
 }
 for(int i=0;i<6;++i)assert(!overlaps(Hotbar(i),Shovel));
 for(bool pool:{false,true})for(int row=0;row<(pool?6:5);++row)for(int col=0;col<9;++col){
   const int x=WorldOffset+40+col*80+40,y=80+row*(pool?85:100)+40;
   assert(x>=SidebarWidth);assert(Cell(x-WorldOffset,y,pool)==row*9+col);
 }
 assert(Cell(SidebarWidth-1-WorldOffset,150,false)==-1);
 assert(Cell(CanvasWidth-1-WorldOffset,150,false)==-1);
 std::cout<<"Native sidebar geometry and all 99 grass/pool cell mappings passed\n";
}
