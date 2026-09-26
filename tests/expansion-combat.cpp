#include "Engine.h"
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "SandboxCombatRules.h"
#include "SandboxArt.h"
#include "SandboxRules.h"
#include "SandboxUIRules.h"
#include "SandboxVisualRules.h"
#include <cassert>
#include <cmath>
#include <iostream>

LawnApp app;LawnApp* gLawnApp=&app;bool gSandboxEnabled=true;
namespace SandboxArt {
Sexy::Image* Image(const char*,const char*){return nullptr;}
void DrawFit(Sexy::Graphics*,Sexy::MemoryImage*,int,int,int,int,float){}
void Sprite(Sexy::Graphics*,const char*,float,float,float,float,float,int){}
void Link(Sexy::Graphics*,float,float,float,float,int,int){}
bool TrackPoint(Reanimation*,const char*,float,float,float,float,float&,float&){return false;}
}

struct World:Board {
    World(){SandboxPlants::Reset();SandboxZombies::Reset();}
    Plant* add(int id,int col=1,int row=2){
        auto* p=plant(col,row);p->mSeedType=static_cast<SeedType>(SandboxPlants::Base(id));SandboxPlants::Assign(p,id);return p;
    }
    Zombie* enemy(int id=0,float x=500,int row=2){
        auto* z=AddZombieInRow(static_cast<ZombieType>(SandboxZombies::Base(id)),row,-1);z->mPosX=x;z->mX=int(x);SandboxZombies::Assign(z,id);return z;
    }
    Projectile* fire(Plant* p,Zombie* z){
        auto* shot=AddProjectile(p->mX+60,p->mY+30,0,p->mRow,PROJECTILE_PEA);SandboxPlants::OnFired(p,shot,z);return shot;
    }
    void step(int n=1){for(int i=0;i<n;++i){SandboxPlants::Tick(this);SandboxZombies::Tick(this);if(!mPaused)++mMainCounter;}}
};

int main(){
    // Roster and validation: the old expansion IDs have no hidden compatibility path.
    assert(SandboxPlants::Definitions.size()==10&&SandboxZombies::Definitions.size()==10);
    for(int id=100;id<=109;++id)assert(SandboxPlants::Find(id)&&SandboxRules::ValidPlant(id));
    for(int id=200;id<=209;++id)assert(SandboxZombies::Find(id)&&SandboxRules::ValidZombie(id));
    assert(!SandboxPlants::Find(110)&&!SandboxZombies::Find(210));
    assert(!SandboxRules::ValidPlant(110)&&!SandboxRules::ValidZombie(210));
    for(int i=0;i<18;++i){auto b=SandboxUIRules::SidebarPlant(i);assert(b.x>=0&&b.x+b.w<=SandboxUIRules::SidebarWidth&&b.y+b.h<520);}

    // A hub only links orthogonally adjacent technology plants; diagonal orphans stay outside the network.
    {World w;auto* hub=w.add(103,1,2);auto* pod=w.add(101,2,2);auto* arc=w.add(104,2,3);auto* orphan=w.add(107,5,2);SandboxPlants::RebuildRootNetworks(&w);
        const auto view=SandboxPlants::RootNetwork(hub);assert(view.members==3&&view.linked&&view.charge==0);
        assert(SandboxPlants::RootLinked(hub,pod)&&SandboxPlants::RootLinked(pod,arc)&&!SandboxPlants::RootLinked(hub,orphan));
    }
    // Three sources share the same capped meter and a resonance cannot be double-triggered.
    {World w;auto* hub=w.add(103,1,2);auto* pod=w.add(101,2,2);SandboxPlants::RebuildRootNetworks(&w);
        assert(SandboxPlants::AddRootEnergy(&w,pod,SandboxPlants::RootEnergySource::Sun));
        assert(SandboxPlants::AddRootEnergy(&w,pod,SandboxPlants::RootEnergySource::Damage));
        assert(SandboxPlants::AddRootEnergy(&w,pod,SandboxPlants::RootEnergySource::ScrapCore,2));
        assert(SandboxPlants::RootNetwork(hub).charge==3);
        assert(SandboxPlants::TriggerRootResonance(&w,hub)==SandboxPlants::RootTriggerResult::Fired);
        assert(SandboxPlants::RootNetwork(hub).charge==0&&SandboxPlants::RootNetwork(hub).activeTicks>0);
        assert(SandboxPlants::TriggerRootResonance(&w,hub)==SandboxPlants::RootTriggerResult::NotFull);
    }
    // Pause freezes every meter and resonance duration instead of draining them in the background.
    {World w;auto* hub=w.add(103,1,2);auto* pod=w.add(101,2,2);SandboxPlants::RebuildRootNetworks(&w);
        SandboxPlants::AddRootEnergy(&w,pod,SandboxPlants::RootEnergySource::Sun,3);assert(SandboxPlants::TriggerRootResonance(&w,hub)==SandboxPlants::RootTriggerResult::Fired);
        const int before=SandboxPlants::RootNetwork(hub).activeTicks;w.mPaused=true;w.step(80);
        assert(!SandboxPlants::AddRootEnergy(&w,pod,SandboxPlants::RootEnergySource::Sun));
        assert(SandboxPlants::TriggerRootResonance(&w,hub)==SandboxPlants::RootTriggerResult::Paused);
        assert(SandboxPlants::RootNetwork(hub).activeTicks==before);
    }
    // A broken cable isolates its branch. The repair moss restores the exact original graph without free energy.
    {World w;auto* hub=w.add(103,1,2);auto* pod=w.add(101,2,2);auto* shell=w.add(102,3,2);auto* moss=w.add(109,2,3);SandboxPlants::RebuildRootNetworks(&w);
        assert(SandboxPlants::AddRootEnergy(&w,hub,SandboxPlants::RootEnergySource::Sun,2));
        assert(SandboxPlants::SeverRootCable(&w,pod,330));assert(SandboxPlants::RootNetwork(pod).severed&&!SandboxPlants::RootLinked(hub,shell));
        assert(SandboxPlants::RepairRootCable(&w,moss,pod));assert(SandboxPlants::RootLinked(hub,shell));assert(SandboxPlants::RootNetwork(hub).charge==2);
    }
    // Jammer pauses charge only; a filled meter can still be spent as a deliberate player action.
    {World w;auto* hub=w.add(103,1,2);auto* pod=w.add(101,2,2);auto* jam=w.enemy(207,150,2);SandboxPlants::RebuildRootNetworks(&w);
        assert(SandboxPlants::RootNetwork(hub).jammed&&!SandboxPlants::AddRootEnergy(&w,pod,SandboxPlants::RootEnergySource::Sun));
        jam->mDead=true;SandboxPlants::RebuildRootNetworks(&w);assert(SandboxPlants::AddRootEnergy(&w,pod,SandboxPlants::RootEnergySource::Sun,3));
        jam->mDead=false;SandboxPlants::RebuildRootNetworks(&w);assert(SandboxPlants::TriggerRootResonance(&w,hub)==SandboxPlants::RootTriggerResult::Fired);
    }
    // The hook uses the same narrow plant-side API as the real enemy tick.
    {World w;auto* hub=w.add(103,1,2);auto* pod=w.add(101,2,2);auto* hook=w.enemy(203,150,2);SandboxPlants::RebuildRootNetworks(&w);
        assert(SandboxZombies::HookRootCable(&w,hook,pod));assert(SandboxPlants::RootNetwork(pod).severed&&!SandboxPlants::RootLinked(hub,pod));
    }
    // Generator and mechanical scraps have separate acquisition routes, with core credit de-duplicated by ZombieID.
    {World w;auto* hub=w.add(103,1,2);auto* sun=w.add(100,2,2);w.step(210);assert(SandboxPlants::RootNetwork(hub).charge>=1);
        auto* core=w.enemy(200,170,2);const int before=SandboxPlants::RootNetwork(hub).charge;
        SandboxPlants::OnMechanicalCoreDestroyed(&w,core);SandboxPlants::OnMechanicalCoreDestroyed(&w,core);
        assert(SandboxPlants::RootNetwork(hub).charge==std::min(3,before+1));(void)sun;
    }
    // A resonating pulse pod emits the native shot plus two explicitly tracked clones.
    {World w;auto* hub=w.add(103,1,2);auto* pod=w.add(101,2,2);SandboxPlants::RebuildRootNetworks(&w);SandboxPlants::AddRootEnergy(&w,pod,SandboxPlants::RootEnergySource::Sun,3);
        assert(SandboxPlants::TriggerRootResonance(&w,hub)==SandboxPlants::RootTriggerResult::Fired);auto* z=w.enemy();w.fire(pod,z);assert(w.mProjectiles.mSize==3);
    }
    // The five projectile families each own their combat rule rather than using a color swap.
    {World w;auto* arc=w.add(104);auto* a=w.enemy(0,400),*b=w.enemy(0,450),*c=w.enemy(0,490,3);SandboxPlants::Impact(w.fire(arc,a),a);
        assert(a->mBodyHealth<1000&&b->mBodyHealth<1000&&c->mBodyHealth<1000);
    }
    {World w;auto* magnet=w.add(105);auto* shield=w.enemy(201,420);const int armor=shield->mHelmHealth;SandboxPlants::Impact(w.fire(magnet,shield),shield);assert(shield->mHelmHealth<armor);}
    {World w;auto* resin=w.add(106);auto* z=w.enemy();SandboxPlants::Impact(w.fire(resin,z),z);assert(SandboxZombies::IsResinSlowed(z));}
    {World w;auto* scout=w.add(107);auto* dead=w.enemy(0,420),*next=w.enemy(0,500);auto* shot=w.fire(scout,dead);dead->mDead=true;SandboxPlants::UpdateShot(shot);assert(shot->mTargetZombieID==next->id);next->mDead=true;SandboxPlants::UpdateShot(shot);assert(shot->mMotionType==MOTION_STRAIGHT);}
    {World w;auto* reed=w.add(108);auto* a=w.enemy(0,400),*b=w.enemy(0,500);w.step();assert(a->mBodyHealth<1000&&b->mBodyHealth<1000);}
    {World w;auto* shell=w.add(102,1,2);auto* z=w.enemy(0,140,2);w.step();assert(z->mBodyHealth<1000);(void)shell;}

    // Each mechanical enemy exposes a distinct, observable counterplay rule.
    {World w;auto* shield=w.enemy(201);auto* repair=w.enemy(204);assert(shield->mHelmHealth==750&&repair->mHelmHealth==650);}
    {World w;auto* turbine=w.enemy(202);assert(SandboxZombies::Speed(turbine)<1);w.step(420);assert(SandboxZombies::Speed(turbine)>1.8f);}
    {World w;auto* shooter=w.enemy(205,480);w.add(0,1,2);w.step(154);assert(w.mProjectiles.mSize==1&&SandboxZombies::HasShot(w.mProjectiles.values.back()));}
    {World w;auto* drone=w.enemy(206);assert(SandboxZombies::Speed(drone)>1.1f);}
    {World w;auto* magnet=w.enemy(208,400);auto* plant=w.add(101,1,2);auto* shot=w.fire(plant,magnet);shot->mPosX=444;SandboxPlants::UpdateShot(shot);assert(shot->mDead&&!SandboxPlants::HasShot(shot));}
    {World w;auto* decoy=w.enemy(209);w.step(45);assert(SandboxZombies::Damage(decoy,20)==0);assert(SandboxZombies::Damage(decoy,20)==20);w.step(330);assert(SandboxZombies::Damage(decoy,20)==0);}
    {World w;auto* battery=w.enemy(200);const int before=battery->mBodyHealth;assert(SandboxZombies::ArcHit(battery,20));assert(battery->mBodyHealth==before-40);}
    {World w;auto* repair=w.enemy(204,440);auto* shield=w.enemy(201,460);shield->mHelmHealth=700;w.step(300);assert(shield->mHelmHealth>700);(void)repair;}
    {World w;auto* hook=w.enemy(203);assert(!SandboxZombies::HookRootCable(&w,hook,w.add(101)));}
    {World w;auto* jam=w.enemy(207);assert(SandboxZombies::IsMechanical(jam));}

    // Reset removes all instance keyed state; stale enemies and roots cannot influence a new board.
    {World w;auto* hub=w.add(103);auto* pod=w.add(101,2,2);auto* z=w.enemy(208);SandboxPlants::RebuildRootNetworks(&w);SandboxPlants::AddRootEnergy(&w,pod,SandboxPlants::RootEnergySource::Sun);
        SandboxPlants::Reset();SandboxZombies::Reset();assert(SandboxPlants::RootNetwork(hub).network==-1&&SandboxZombies::Speed(z)==1);
    }
    std::cout<<"Tech root-network combat scenarios passed\n";
}
