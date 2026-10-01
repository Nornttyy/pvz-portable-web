#include "ConstEnums.h"
#include "EverythingShooterRules.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
constexpr int FOLEY_PUFF=1,FOLEY_THROW=2,FOLEY_SPLAT=3,FOLEY_CHERRYBOMB=4;
namespace Sexy {int rare=99,native=0;int Rand(int n){return n==100?rare:native;}constexpr int SOUND_DOOMSHROOM=888;struct Color{int r,g,b;Color(int r,int g,int b):r(r),g(g),b(b){}};}
struct Reanimation{};
namespace SandboxArt {bool tracked=false;bool TrackPoint(Reanimation*,const char*,float,float,float,float,float& x,float& y){if(!tracked)return false;x=70;y=40;return true;}}
struct Particle {int colors=0;void OverrideColor(const char*,Sexy::Color c){assert(c.r==120&&c.g==83&&c.b==43);++colors;}};
struct App {int sounds=0,particles=0,effect=-1;Particle puff;Reanimation* ReanimationTryToGet(int){return nullptr;}void PlayFoley(int){++sounds;}void PlaySample(int){++sounds;}Particle* AddPvzpParticle(float,float,int,int e){++particles;effect=e;return &puff;}};
struct Zombie {bool mDead=false,dying=false;int health=6000,hits=0;float x=600;struct Rect{int mY;};Rect GetZombieRect(){return {240};}float ZombieTargetLeadX(int){return x;}bool IsDeadOrDying(){return dying;}void TakeDamage(int n,int flags){assert(flags==1);health-=n;++hits;}};
struct Board;
struct Projectile {Board* mBoard=nullptr;App* mApp=nullptr;bool mDead=false;int style=0,mDamageRangeFlags=0,mRenderOrder=0,mRow=0,mCobTargetRow=0,converted=0;float mPosX=0,mPosY=0,mPosZ=0,mVelX=0,mVelY=0,mVelZ=0,mAccZ=0,mCobTargetX=0;ProjectileType mProjectileType=PROJECTILE_PEA;ProjectileMotion mMotionType=MOTION_STRAIGHT;void ConvertToFireball(int){mProjectileType=PROJECTILE_FIREBALL;++converted;}int GetDamageFlags(Zombie*){return 1;}void Die(){mDead=true;}};
struct Board {struct {int mSize=0,mMaxSize=1024;} mProjectiles;App app;Projectile last;int shots=0,blasts=0,shakes=0,row=-1,radius=0,rows=0,flags=0;float x=0,y=0;bool burn=false;
 Projectile* AddProjectile(float x,float y,int order,int row,ProjectileType type){assert(type!=PROJECTILE_FIREBALL);last={};last.mBoard=this;last.mApp=&app;last.mProjectileType=type;last.mPosX=x;last.mPosY=y;last.mRow=row;last.mRenderOrder=order;++shots;return &last;}
 void KillAllZombiesInRadius(int r,float px,float py,int range,int lanes,bool b,int f){++blasts;row=r;x=px;y=py;radius=range;rows=lanes;burn=b;flags=f;}
 void ShakeBoard(int,int){++shakes;}
};
struct Plant {Board* mBoard=nullptr;App* mApp=nullptr;int id=529,mX=120,mY=250,mRow=2,mRenderOrder=100,mHeadReanimID=1,mPlantCol=1;};
bool gSandboxEnabled=false;
namespace MemeAdventure {bool enabled=false;bool RosterEnabled(){return enabled;}}
namespace MemeCharacters {int Type(const Plant* p){return p?p->id:0;}int ShotStyle(const Projectile* p){return p->style;}bool RestoreShotStyle(Projectile* p,int s){assert(EverythingShooterRules::Valid(s,p->mProjectileType,p->mMotionType));p->style=s;return true;}}
namespace EverythingShooter {int fired[17]{},impacts[17]{};
#include "everything-production.inc"
}
int main(){
 using namespace EverythingShooterRules;
 static_assert(Cost==250&&Recharge==750&&Interval==150&&Unlock==27&&Base==49&&Id==529);
 for(bool sandbox:{false,true})for(bool adventure:{false,true}){gSandboxEnabled=sandbox;MemeAdventure::enabled=adventure;for(int seed=0;seed<54;++seed)assert(EverythingShooter::IsSlot(seed)==(seed==49&&(sandbox||adventure)));}
 std::array<int,17> distribution{};
 for(int roll=0;roll<100;++roll)for(int native=0;native<14;++native){
  const int style=Choose(roll,native);++distribution[style-First];Sexy::rare=roll;Sexy::native=native;
  Board b;Plant p{&b,&b.app};Zombie z;SandboxArt::tracked=roll%2;
  assert(EverythingShooter::Fire(&p,&z)&&b.shots==1);auto& shot=b.last;
  assert(shot.style==style&&shot.mProjectileType==NativeType(style)&&shot.mRow==2&&shot.mRenderOrder==99);
  assert(shot.mPosX==120+(SandboxArt::tracked?70:58)-12&&shot.mPosY==250+(SandboxArt::tracked?40:34)-12);
  assert(shot.mMotionType==(Lob(style)?MOTION_LOBBED:NativeType(style)==PROJECTILE_PUFF?MOTION_PUFF:MOTION_STRAIGHT));
  assert(shot.converted==(NativeType(style)==PROJECTILE_FIREBALL));
  assert(shot.mDamageRangeFlags==(Special(style)||NativeType(style)==PROJECTILE_COBBIG?127:Lob(style)?13:1));
  if(NativeType(style)==PROJECTILE_COBBIG)assert(shot.mVelZ==-8&&shot.mAccZ==0&&shot.mCobTargetRow==2&&shot.mCobTargetX==520);
  else if(Lob(style))assert(shot.mVelX>0&&shot.mVelZ<0&&shot.mAccZ>.11&&shot.mAccZ<.12);
  else assert(std::abs(shot.mVelX-3.33f)<.0001&&shot.mVelY==0);
  assert(EverythingShooter::Impact(&shot,&z)==Special(style));
  if(!Special(style))assert(!shot.mDead&&z.health==6000&&!b.blasts&&!b.app.particles);
  else if(style==Poop)assert(shot.mDead&&z.health==5920&&z.hits==1&&!b.blasts&&b.app.effect==PARTICLE_PUFF_SPLAT&&b.app.puff.colors==1);
  else assert(shot.mDead&&b.blasts==1&&b.row==2&&b.radius==(style==Doom?250:115)&&b.rows==(style==Doom?3:1)&&b.burn&&b.flags==127&&b.shakes==1&&b.app.effect==(style==Doom?PARTICLE_DOOM:PARTICLE_POWIE));
 }
 for(int i=0;i<14;++i)assert(distribution[i]==80);
 assert(distribution[14]==70&&distribution[15]==70&&distribution[16]==140);
 for(int style=First;style<=Poop;++style){const int type=NativeType(style),motion=Lob(style)?1:type==4?5:0;assert(Valid(style,type,motion));assert(!Valid(style,type,(motion+1)%10));if(type==0||type==1)assert(Valid(style,6,motion));else assert(!Valid(style,(type+1)%14,motion));}
 assert(!Valid(319,0,0)&&!Valid(337,0,0));
 {Board b;Plant p{&b,&b.app};Zombie z;p.id=500;assert(!EverythingShooter::Fire(&p,&z)&&!b.shots);p.id=529;assert(EverythingShooter::Fire(&p,nullptr)&&!b.shots);z.mDead=true;assert(EverythingShooter::Fire(&p,&z)&&!b.shots);z.mDead=false;b.mProjectiles.mSize=1016;assert(EverythingShooter::Fire(&p,&z)&&!b.shots);p.mBoard=nullptr;assert(EverythingShooter::Fire(&p,&z));assert(!EverythingShooter::Fire(nullptr,&z));}
 {Board b;auto* shot=b.AddProjectile(10,20,0,2,PROJECTILE_PEA);Zombie z;assert(!EverythingShooter::Impact(shot,&z)&&!shot->mDead);shot->style=Poop;z.mDead=true;assert(EverythingShooter::Impact(shot,&z)&&z.health==6000);}
 std::cout<<"all 17 production launches, exact weights, native effects, muzzle anchors, capacity and rare impacts passed\n";
}
