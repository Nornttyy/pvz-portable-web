#include "IceChiliRules.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <vector>
constexpr int FOLEY_FROZEN=1,RENDER_LAYER_PARTICLE=2,PARTICLE_ICE_TRAP_RELEASE=3,ZOMBIE_BALLOON=16;
struct Zombie {
 bool mDead=false,mMindControlled=false,onboard=true,dying=false,eligible=true,freezable=true;
 int mRow=2,mBodyHealth=6000,mIceTrapCounter=0,mChilledCounter=0,mZombieType=0,hits=0,stops=0,anim=0;
 bool spin=true;
 bool IsOnBoard(){return onboard;}bool IsDeadOrDying(){return dying||mDead||mBodyHealth<=0;}
 bool EffectedByDamage(unsigned flags){assert(flags==127);return eligible;}
 bool CanBeFrozen(){return freezable;}
 void TakeDamage(int n,unsigned flags){assert(flags==1);mBodyHealth-=n;++hits;}
 void StopZombieSound(){++stops;}void BalloonPropellerHatSpin(bool value){spin=value;}void UpdateAnimSpeed(){++anim;}
};
struct App {
 std::vector<std::array<int,4>> particles;int sounds=0;
 void PlayFoley(int f){assert(f==FOLEY_FROZEN);++sounds;}
 void AddPvzpParticle(float x,float y,int order,int effect){particles.push_back({int(x),int(y),order,effect});}
};
struct Board {
 App app;App* mApp=&app;std::vector<Zombie*> mZombies;int shakes=0,map=0;
 int GridToPixelX(int col,int){return 40+80*col;}
 int GetPosYBasedOnRow(float x,int row){return 80+row*(map==1?85:100)+(map==2?int((720-x)*.1f):0);}
 static int MakeRenderOrder(int layer,int row,int offset){return layer*100+row*10+offset;}
 void ShakeBoard(int x,int y){assert(x==2&&y==-2);++shakes;}
};
struct Plant {Board* mBoard=nullptr;bool mDead=false;int mRow=2,id=528;void Die(){mDead=true;}};
namespace MemeCharacters {int Type(const Plant* p){return p?p->id:0;}}
#include "ice-production.inc"
int main(){
 static_assert(IceChiliRules::Cost==150&&IceChiliRules::Recharge==5000&&IceChiliRules::Windup==100&&IceChiliRules::Unlock==26);
 for(int map=0;map<3;++map)for(int row=0;row<(map==1?6:5);++row){
  Board b;b.map=map;Plant p{&b,false,row};std::array<Zombie,12> zs;
  for(auto& z:zs){z.mRow=row;b.mZombies.push_back(&z);}
  zs[1].mRow=(row+1)%5;zs[2].onboard=false;zs[3].mMindControlled=true;zs[4].mDead=true;zs[5].dying=true;zs[6].eligible=false;
  zs[7].freezable=false;zs[8].mBodyHealth=270;zs[9].mIceTrapCounter=500;zs[9].mChilledCounter=1700;
  zs[10].mIceTrapCounter=50;zs[11].mZombieType=ZOMBIE_BALLOON;
  assert(IceChili::Detonate(&p)&&p.mDead);assert(!IceChili::Detonate(&p));
  for(int i:{0,7,8,9,10,11})assert(zs[i].hits==1);
  for(int i:{1,2,3,4,5,6})assert(!zs[i].hits&&zs[i].mBodyHealth==6000&&!zs[i].mIceTrapCounter);
  for(int i:{0,10,11})assert(zs[i].mBodyHealth==4800&&zs[i].mIceTrapCounter==300&&!zs[i].mChilledCounter&&zs[i].stops==1&&zs[i].anim==1);
  assert(zs[7].mBodyHealth==4800&&!zs[7].mIceTrapCounter&&!zs[7].stops);
  assert(zs[8].mBodyHealth==270-1200&&!zs[8].mIceTrapCounter);
  assert(zs[9].mBodyHealth==4800&&zs[9].mIceTrapCounter==500&&zs[9].mChilledCounter==1700);
  assert(!zs[11].spin&&b.app.sounds==1&&b.shakes==1&&b.app.particles.size()==9);
  for(int col=0;col<9;++col){const auto burst=b.app.particles[col];assert(burst[0]==80+80*col&&burst[1]==b.GetPosYBasedOnRow(burst[0],row)+60&&burst[2]==Board::MakeRenderOrder(RENDER_LAYER_PARTICLE,row,1)&&burst[3]==PARTICLE_ICE_TRAP_RELEASE);}
 }
 Board b;Plant ordinary{&b,false,2,20};assert(!IceChili::Detonate(&ordinary)&&!ordinary.mDead&&b.app.particles.empty());
 Plant empty;assert(!IceChili::Detonate(&empty)&&!IceChili::Detonate(nullptr));
 std::cout<<"same-row 1200 damage, 300-tick freeze, original exclusions and nine native ice bursts passed\n";
}
