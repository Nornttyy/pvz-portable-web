#include "Engine.h"
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "SandboxCombatRules.h"
#include "SandboxArt.h"
#include "SandboxRules.h"
#include "SandboxUIRules.h"
#include "SandboxVisualRules.h"
#include <cassert>
#include <iostream>
#include <cmath>
LawnApp app;LawnApp* gLawnApp=&app;bool gSandboxEnabled=true;
namespace SandboxArt {
Sexy::Image* Image(const char*,const char*){return nullptr;}void DrawFit(Sexy::Graphics*,Sexy::MemoryImage*,int,int,int,int,float){}
void Sprite(Sexy::Graphics*,const char*,float,float,float,float,float,int){}
void Link(Sexy::Graphics*,float,float,float,float,int,int){}
bool TrackPoint(Reanimation*,const char*,float,float,float,float,float&,float&){return false;}
}
struct World:Board{
 World(){SandboxPlants::Reset();SandboxZombies::Reset();}
 Plant* add(int id,int col=1,int row=2){auto* p=plant(col,row);p->mSeedType=static_cast<SeedType>(SandboxPlants::Base(id));SandboxPlants::Assign(p,id);return p;}
 Zombie* enemy(int id=0,float x=500,int row=2){auto* z=AddZombieInRow(static_cast<ZombieType>(SandboxZombies::Base(id)),row,-1);z->mPosX=x;SandboxZombies::Assign(z,id);return z;}
 Projectile* fire(Plant* p,Zombie* z){auto* s=AddProjectile(p->mX+60,p->mY+30,0,p->mRow,PROJECTILE_PEA);SandboxPlants::OnFired(p,s,z);return s;}
 void step(int n=1){for(int i=0;i<n;++i){SandboxPlants::Tick(this);SandboxZombies::Tick(this);if(!mPaused)++mMainCounter;}}
};
int main(){
 for(bool slow:{false,true}){int counter=150,at25=0,at50=0,tick=0;while(counter>0){if(slow&&tick%3==0&&SandboxCombatRules::CanSlow(counter))++counter;else if(!slow&&tick%2==0&&SandboxCombatRules::CanHaste(counter))--counter;--counter;if(counter==25)++at25;if(counter==50)++at50;++tick;}assert(at25==1&&at50==1);}
 assert(SandboxPlants::Definitions.size()==17);assert(SandboxZombies::Definitions.size()==12);
 for(int i=108;i<118;++i)assert(SandboxRules::ValidPlant(i)==(i!=109));assert(!SandboxRules::ValidPlant(118));
 for(int i=200;i<212;++i)assert(SandboxRules::ValidZombie(i));assert(!SandboxRules::ValidZombie(212));
 for(int i=0;i<18;++i){auto b=SandboxUIRules::SidebarPlant(i);assert(b.x>=0&&b.x+b.w<=SandboxUIRules::SidebarWidth&&b.y+b.h<520);}
 {World w;auto* p=w.add(107);assert(SandboxPlants::NextShot(p)==1);assert(SandboxPlants::NextShot(p)==2);assert(SandboxPlants::NextShot(p)==1);}
 {World w;auto* p=w.add(113);assert(p->mLaunchRate==75);assert(SandboxPlants::NextShot(p)==0);float x=0,y=0,sx=1,sy=1;SandboxPlants::AdjustScale(p,x,y,sx,sy);assert(std::abs(sx-0.72)<0.001&&std::abs(y-18.2)<0.01);auto* z=w.enemy();auto* shot=w.fire(p,z);assert(SandboxPlants::Impact(shot,z));assert(z->mBodyHealth==990);SandboxPlants::ForgetShot(shot);assert(!SandboxPlants::Impact(shot,z));}
 {World w;auto* p=w.add(114);auto* z=w.enemy();SandboxPlants::Impact(w.fire(p,z),z);assert(z->mBodyHealth==935&&z->mPosX==535);}
 {World w;auto* p=w.add(115);w.fire(p,nullptr);assert(w.mProjectiles.mSize==3);assert(w.mProjectiles.values[1]->mRow==1&&w.mProjectiles.values[2]->mRow==3);World edge;edge.fire(edge.add(115,1,0),nullptr);assert(edge.mProjectiles.mSize==2);}
 {World w;auto* p=w.add(116);auto* z=w.enemy();auto* next=w.enemy(0,520,3);auto* s=w.fire(p,z);assert(s->mTargetZombieID==z->id);z->mDead=true;SandboxPlants::UpdateShot(s);assert(s->mTargetZombieID==next->id);next->mDead=true;SandboxPlants::UpdateShot(s);assert(s->mMotionType==MOTION_STRAIGHT);}
 {World w;auto* p=w.add(112);auto* a=w.enemy(0,400,2);auto* b=w.enemy(0,450,2);auto* c=w.enemy(0,490,3);auto* outside=w.enemy(0,750,2);SandboxPlants::Impact(w.fire(p,a),a);assert(a->mBodyHealth==980&&b->mBodyHealth==986&&c->mBodyHealth==992&&outside->mBodyHealth==1000);}
 {World w;auto* p=w.add(117);auto* z=w.enemy();SandboxPlants::Impact(w.fire(p,z),z);w.mPaused=true;w.step(500);assert(z->mBodyHealth==988);w.mPaused=false;w.step(500);assert(z->mBodyHealth==963);w.step(300);assert(z->mBodyHealth==963);}
 {World w;auto* p=w.add(117);auto* z=w.enemy();SandboxPlants::Impact(w.fire(p,z),z);w.step(200);SandboxPlants::Impact(w.fire(p,z),z);w.step(500);assert(z->mBodyHealth==941);}
 {World w;assert(!SandboxPlants::Find(109));auto* p=w.add(111);auto* z=w.enemy(0,400);w.step(104);assert(w.mProjectiles.mSize==1);auto* s=w.mProjectiles.values.back();assert(SandboxPlants::HasShot(s));SandboxPlants::Impact(s,z);assert(z->mBodyHealth==976);}
 {World w;w.add(108);auto* a=w.enemy(0,500),*b=w.enemy(0,600),*other=w.enemy(0,500,1);w.step(305);assert(a->mBodyHealth==964&&b->mBodyHealth==964&&other->mBodyHealth==1000);}
 {World w;auto* p=w.add(0);p->mLaunchCounter=100;w.add(110,0,2);w.add(110,2,2);w.step();assert(p->mLaunchCounter==99);w.mPaused=true;w.step(100);assert(p->mLaunchCounter==99);}
 {World w;auto* p=w.add(111);w.enemy(0,700);w.step(1499);assert(w.mProjectiles.mSize==0&&SandboxPlants::GrowthStage(p)==0);w.mPaused=true;w.step(1000);assert(SandboxPlants::GrowthStage(p)==0);w.mPaused=false;w.step();assert(SandboxPlants::GrowthStage(p)==1);p->mIsAsleep=true;w.step(5000);assert(SandboxPlants::GrowthStage(p)==1);p->mIsAsleep=false;w.step(2000);assert(SandboxPlants::GrowthStage(p)==2);}
 {World w;auto* z=w.enemy(200);z->mHelmHealth=0;w.step();assert(SandboxZombies::Speed(z)>1.7);w.mPaused=true;w.step(500);assert(SandboxZombies::Speed(z)>1.7);w.mPaused=false;w.step(300);assert(SandboxZombies::Speed(z)==1);w.step();assert(SandboxZombies::Speed(z)==1);}
 {World w;auto* a=w.enemy(201,400),*b=w.enemy(201,450),*z=w.enemy(0,430);w.step();assert(std::abs(SandboxZombies::Speed(z)-1.35)<0.001);a->mDead=b->mDead=true;w.step();assert(SandboxZombies::Speed(z)==1);}
 {World w;auto* p=w.add(0);w.enemy(202,100);w.enemy(202,120);w.step();assert(p->mLaunchCounter==101);}
 {World w;auto* z=w.enemy(203);z->chill=100;w.step();assert(z->chill==0);z->mHelmHealth=0;z->chill=100;w.step();assert(z->chill==100);}
 {World w;auto* z=w.enemy(204);z->mBodyHealth-=100;assert(SandboxZombies::ElectricHit(z));assert(z->mBodyHealth==320);assert(!SandboxZombies::ElectricHit(z));assert(SandboxZombies::Speed(z)>1.3);}
 {World w;auto* small=w.enemy(205),*big=w.enemy(206);assert(small->mBodyHealth==160&&small->mScaleZombie<1&&SandboxZombies::Speed(small)>1.6);assert(big->mHelmHealth==950&&SandboxZombies::Speed(big)<0.7);}
 {World w;auto* repair=w.enemy(207,450),*target=w.enemy(206,460);target->mHelmHealth=940;w.step(400);assert(target->mHelmHealth==950);target->mHelmHealth=0;w.step(400);assert(target->mHelmHealth==0);(void)repair;}
 {World w;auto* z=w.enemy(208);assert(SandboxZombies::Damage(z,21)==11);assert(SandboxZombies::Damage(z,0)==0);w.step(240);assert(SandboxZombies::Damage(z,21)==21);}
 {World w;auto* z=w.enemy(209);SandboxZombies::CombatDeath(z);SandboxZombies::CombatDeath(z);w.step();assert(w.mZombies.mSize==3);w.step();assert(w.mZombies.mSize==3);}
 {World w;auto* z=w.enemy(209);SandboxZombies::CombatDeath(z);SandboxZombies::Reset();w.step();assert(w.mZombies.mSize==1);}
 {World w;auto* z=w.enemy(209);for(int i=1;i<160;++i)w.enemy();SandboxZombies::CombatDeath(z);w.step();assert(w.mZombies.mSize==160);}
 {World w;auto* p=w.add(117);auto* z=w.enemy();auto* shot=w.fire(p,z);SandboxPlants::Impact(shot,z);SandboxPlants::Reset();w.step(500);assert(z->mBodyHealth==988);assert(!SandboxPlants::Impact(shot,z));}
 {World w;auto* p=w.add(111);auto* z=w.enemy(0,350),*next=w.enemy(0,390);w.step(3700);auto* s=w.mProjectiles.values.back();const int before=z->mBodyHealth;SandboxPlants::Impact(s,z);assert(z->mBodyHealth==before-55&&next->mBodyHealth==951);s->mPosX=9999;SandboxPlants::UpdateShot(s);assert(s->mDead&&!SandboxPlants::HasShot(s));(void)p;}
 {World w;auto* p=w.add(0);auto* z=w.enemy(210,400);w.step(120);assert(SandboxZombies::Speed(z)==0&&w.mProjectiles.mSize==0);w.mPaused=true;w.step(500);assert(w.mProjectiles.mSize==0);w.mPaused=false;w.step(70);assert(w.mProjectiles.mSize==1&&SandboxZombies::Speed(z)>0);auto* s=w.mProjectiles.values.back();assert(s->mMotionType==MOTION_BACKWARDS&&s->mProjectileType==PROJECTILE_ZOMBIE_PEA);assert(p->mPlantHealth==300);assert(SandboxZombies::Impact(s,p));assert(p->mPlantHealth==270);s->Die();assert(!SandboxZombies::HasShot(s));}
 {World w;w.add(0);auto* z=w.enemy(210,400);w.step(120);z->mButteredCounter=100;w.step(200);assert(w.mProjectiles.mSize==0);z->mButteredCounter=0;z->mHasArm=false;w.step(1000);assert(w.mProjectiles.mSize==0&&SandboxZombies::Speed(z)>0);}
 {World w;auto* rear=w.add(0,1);auto* front=w.add(3,2);auto* shell=w.add(30,2);w.enemy(210,400);w.step(190);auto* s=w.mProjectiles.values.back();s->mPosX=front->mX+70;assert(SandboxZombies::CollisionTarget(s)==shell);shell->mDead=true;assert(SandboxZombies::CollisionTarget(s)==front);front->mDead=true;assert(!SandboxZombies::CollisionTarget(s));s->mPosX=rear->mX+70;assert(SandboxZombies::CollisionTarget(s)==rear);s->mRow=1;assert(!SandboxZombies::CollisionTarget(s));}
 {World w;auto* p=w.add(0);w.enemy(211,400);w.step(220);assert(w.mProjectiles.mSize==1);auto* s=w.mProjectiles.values.back();SandboxZombies::Impact(s,p);assert(p->mPlantHealth==288&&SandboxZombies::AttackSlowed(p));w.mPaused=true;w.step(1000);assert(SandboxZombies::AttackSlowed(p));w.mPaused=false;w.step(240);assert(!SandboxZombies::AttackSlowed(p));}
 {World w;w.add(0);auto* z=w.enemy(210,400);z->mMindControlled=true;w.step(1000);assert(w.mProjectiles.mSize==0);z->mMindControlled=false;w.mProjectiles.mSize=w.mProjectiles.mMaxSize-8;w.step(190);assert(w.ownedShots.empty());SandboxZombies::Reset();assert(SandboxZombies::Speed(z)==1);}
 std::cout<<"30 production combat scenarios passed; growth, ranged projectiles, pause, cleanup, caps, roles and IDs verified\n";
}
