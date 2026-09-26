// Executes production draw hooks against the raster capture stub.  It verifies
// bone ownership and coordinate contracts, not subjective art quality.
#include "Engine.h"
#include "SandboxArt.h"
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "SandboxVisualRules.h"
#include <cassert>
#include <cmath>
#include <iostream>

LawnApp app;LawnApp* gLawnApp=&app;bool gSandboxEnabled=true;
void near(float a,float b){assert(std::abs(a-b)<0.01f);}

int main(){
    // A regular plant owns one complete custom head.  The borrowed native
    // mouth pivot visibly separates in shooting frames, so it must remain
    // hidden rather than leaving a floating legacy nozzle under the head.
    Board board;
    Track plantTracks[]={{"anim_face"},{"idle_mouth"},{"anim_blink"}};
    TrackInstance plantInstances[3];Reanimation plantRig;plantRig.def.mTracks={3,plantTracks};plantRig.mTrackInstances=plantInstances;app.reanims[90]=&plantRig;
    auto* pod=board.plant(1,2);pod->mSeedType=static_cast<SeedType>(0);pod->mHeadReanimID=90;SandboxPlants::Assign(pod,101);
    assert(plantInstances[0].mImageOverride->path.ends_with("pulse-pod-head.png"));
    assert(plantInstances[1].mRenderGroup==RENDER_GROUP_HIDDEN);
    assert(plantInstances[2].mRenderGroup==RENDER_GROUP_HIDDEN);
    pod->mBlinkCountdown=4;SandboxPlants::Tick(&board);
    assert(plantInstances[0].mImageOverride->path.ends_with("pulse-pod-blink.png"));

    // Three independent Arc Orchid heads retain their own face bones, while
    // all three incompatible legacy mouth pivots remain hidden.
    Track tripleTracks[]={{"anim_face1"},{"anim_face2"},{"anim_face3"},{"ThreePeater_mouth1"},{"ThreePeater_mouth2"},{"ThreePeater_mouth3"}};
    TrackInstance tripleInstances[6];Reanimation tripleRig;tripleRig.def.mTracks={6,tripleTracks};tripleRig.mTrackInstances=tripleInstances;app.reanims[91]=&tripleRig;
    auto* orchid=board.plant(2,2);orchid->mSeedType=static_cast<SeedType>(18);orchid->mHeadReanimID=91;orchid->mHeadReanimID2=91;orchid->mHeadReanimID3=91;SandboxPlants::Assign(orchid,104);
    for(int i=0;i<3;++i)assert(tripleInstances[i].mImageOverride->path.ends_with("arc-orchid-small-head.png"));
    for(int i=3;i<6;++i)assert(tripleInstances[i].mRenderGroup==RENDER_GROUP_HIDDEN);

    // Each zombie has its own head, jaw, torso, four arm seams, hand and all
    // six walking-leg seams. The
    // custom head owns its own headset/hair, so the native hair bone is hidden
    // rather than drawing a pasted legacy wig over the new silhouette.
    Track zombieTracks[]={{"anim_head1"},{"anim_head2"},{"Zombie_body"},{"anim_innerarm1"},{"anim_innerarm2"},{"Zombie_outerarm_upper"},{"Zombie_outerarm_lower"},{"Zombie_outerarm_hand"},{"Zombie_innerleg_upper"},{"Zombie_innerleg_lower"},{"Zombie_innerleg_foot"},{"Zombie_outerleg_upper"},{"Zombie_outerleg_lower"},{"Zombie_outerleg_foot"},{"anim_hair"},{"Zombie_tie"},{"anim_cone"},{"anim_bucket"}};
    TrackInstance zombieInstances[18];Reanimation zombieRig;zombieRig.def.mTracks={18,zombieTracks};zombieRig.mTrackInstances=zombieInstances;app.reanims[92]=&zombieRig;
    for(const auto& d:SandboxZombies::Definitions){
        for(auto& track:zombieInstances){track.mImageOverride=nullptr;track.mRenderGroup=0;}
        auto* z=board.AddZombieInRow(ZOMBIE_NORMAL,2,0);z->mBodyReanimID=92;SandboxZombies::Assign(z,d.id);
        for(int i=0;i<=13;++i)assert(zombieInstances[i].mImageOverride);
        assert(zombieInstances[14].mRenderGroup==RENDER_GROUP_HIDDEN);
        if(d.id==202)assert(zombieInstances[15].mRenderGroup==RENDER_GROUP_HIDDEN);else assert(zombieInstances[15].mImageOverride);
        z->mBodyHealth=z->mBodyMaxHealth/2;z->mHasArm=false;SandboxZombies::RefreshDamageArt(z);
        assert(zombieInstances[2].mImageOverride->path.ends_with("-body-damage1.png"));
        assert(zombieInstances[5].mImageOverride->path.ends_with("-outer-upper-damaged.png"));
        assert(zombieInstances[5].mRenderGroup!=RENDER_GROUP_HIDDEN);
        assert(zombieInstances[6].mRenderGroup==RENDER_GROUP_HIDDEN);
        assert(zombieInstances[7].mRenderGroup==RENDER_GROUP_HIDDEN);
        if(d.armor){
            const int prop=d.base==2?16:17;z->mHelmHealth=d.armor/2;SandboxZombies::RefreshDamageArt(z);
            assert(zombieInstances[prop].mImageOverride->path.ends_with("-prop-damage1.png"));
            assert(SandboxZombies::DetachedArmor(z)->path.ends_with("-prop-damage2.png"));
        }else assert(!SandboxZombies::DetachedArmor(z));
    }
    SandboxZombies::Reset();app.reanims.clear();

    // Sprite drawing translates once and the muzzle follows the live head
    // bone's front edge, not a detached legacy mouth pivot.
    Sexy::Graphics g(nullptr);g.mTransX=170;g.mTransY=250;SandboxArt::Sprite(&g,"pulse",12,12,22,22);near(testBlits.back().matrix.m02,182);near(testBlits.back().matrix.m12,262);
    Reanimation face;face.track="anim_face";face.matrix={0,-0.72f,60,0.72f,0,40};float x=0,y=0;
    assert(SandboxArt::TrackPoint(&face,"anim_face",70,65,67,32.5f,x,y));near(x,60);near(y,63.04f);
    app.reanims[1]=&face;auto* muzzle=board.plant(3,1);muzzle->mHeadReanimID=1;SandboxPlants::Assign(muzzle,101);
    auto* shot=board.AddProjectile(0,0,0,1,PROJECTILE_PEA);SandboxPlants::OnFired(muzzle,shot,nullptr);
    near(shot->mPosX+12,muzzle->mX+60);near(shot->mPosY+12,muzzle->mY+63.04f);assert(SandboxPlants::ShotRadius(shot)==12);
    g.mTransX=shot->mX;g.mTransY=shot->mY;testBlits.clear();assert(SandboxPlants::DrawShot(&g,shot));assert(testBlits.back().path.ends_with("vfx-pulse.png"));

    // Root links and hit effects render only in their own row and clear on reset.
    auto* hub=board.plant(1,3);SandboxPlants::Assign(hub,103);auto* linked=board.plant(2,3);SandboxPlants::Assign(linked,101);
    SandboxPlants::RebuildRootNetworks(&board);testBlits.clear();SandboxPlants::DrawEffects(&g,&board,3);assert(!testBlits.empty());
    auto* enemy=board.AddZombieInRow(ZOMBIE_NORMAL,3,0);auto* hit=board.AddProjectile(150,340,0,3,PROJECTILE_PEA);SandboxPlants::OnFired(linked,hit,enemy);SandboxPlants::Impact(hit,enemy);
    testBlits.clear();SandboxPlants::DrawEffects(&g,&board,2);assert(testBlits.empty());SandboxPlants::DrawEffects(&g,&board,3);assert(!testBlits.empty());
    SandboxPlants::Reset();testBlits.clear();SandboxPlants::DrawEffects(&g,&board,3);assert(testBlits.empty());

    std::cout<<"Visual coordinate contracts passed\n";
}
