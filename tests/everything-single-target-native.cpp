#include "ConstEnums.h"
#include "EverythingShooterRules.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
struct Zombie {
 bool mDead=false,dying=false,frozen=false;int health=6000,hits=0,flags=0;
 bool IsDeadOrDying(){return dying;}
 void TakeDamage(int n,unsigned f){health-=n;++hits;flags=f;frozen=(f&4)!=0;}
};
struct Board {std::vector<Zombie*> mZombies;};
struct ProjectileDefinition {int mDamage;};
struct Projectile {
 Board* mBoard;int style=0;ProjectileType mProjectileType=PROJECTILE_MELON;int splashTests=0;
 const ProjectileDefinition& GetProjectileDef(){static ProjectileDefinition d;d.mDamage=mProjectileType==PROJECTILE_MELON||mProjectileType==PROJECTILE_WINTERMELON?80:mProjectileType==PROJECTILE_COBBIG?300:40;return d;}
 unsigned GetDamageFlags(Zombie*){return mProjectileType==PROJECTILE_WINTERMELON?6:2;}
 bool IsZombieHitBySplash(Zombie*){++splashTests;return true;}
 void DoSplashDamage(Zombie*);void ApplyDirect(Zombie*);
};
namespace MemeCharacters {int ShotStyle(const Projectile* p){return p->style;}}
namespace SandboxPlants {int ShotDamage(const Projectile*,int damage){return damage;}}
#include "single-production.inc"
int main(){
 using namespace EverythingShooterRules;
 for(auto type:{PROJECTILE_MELON,PROJECTILE_WINTERMELON,PROJECTILE_FIREBALL}){
  // Include ordinary/snow peas converted by torchwood and restored shot IDs.
  for(int style:{First,First+1,First+3,First+5,First+6}){
   Board b;Zombie target,neighbor,adjacent;b.mZombies={&target,&neighbor,&adjacent};Projectile shot{&b,style,type};
   shot.DoSplashDamage(&target);assert(target.hits==1&&target.health==6000-shot.GetProjectileDef().mDamage);
   assert(target.frozen==(type==PROJECTILE_WINTERMELON));assert(neighbor.hits==0&&adjacent.hits==0&&shot.splashTests==0);
   shot.DoSplashDamage(nullptr);target.mDead=true;shot.DoSplashDamage(&target);target.mDead=false;target.dying=true;shot.DoSplashDamage(&target);
   assert(target.hits==1&&neighbor.hits==0&&adjacent.hits==0);
  }
  Board b;Zombie target,neighbor,adjacent;b.mZombies={&target,&neighbor,&adjacent};Projectile original{&b,0,type};
  original.DoSplashDamage(&target);assert(target.hits==1&&neighbor.hits==1&&adjacent.hits==1&&original.splashTests>0);
 }
 for(int style:{0,Cob}){Board b;Zombie target,neighbor;b.mZombies={&target,&neighbor};Projectile shot{&b,style,PROJECTILE_COBBIG};shot.ApplyDirect(&target);
  assert(target.health==6000-(style==Cob?600:300)&&target.hits==1&&neighbor.hits==0);assert(target.flags==(style==Cob?18:2));}
 std::cout<<"single target only, native area retained\n";
}
