// Runs actual production skin/projectile/effect draw code against a captured raster API.
// It checks coordinate contracts, not a live browser/GPU screenshot.
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
 // Actual production Skin() is exercised through Assign, including stale override removal.
 Board skinBoard;
 Track tracks[]={{"anim_face"},{"GatlingPea_mouth"},{"GatlingPea_barrel1"},{"GatlingPea_barrel2"},{"GatlingPea_barrel3"},{"GatlingPea_barrel4"},{"GatlingPea_mouth_overlay"},{"GatlingPea_helmet"},{"unrelated_mouth_decoration"}};
 TrackInstance instances[9];Reanimation skin;skin.def.mTracks={9,tracks};skin.mTrackInstances=instances;
 Sexy::Image stale;for(auto& t:instances){t.mImageOverride=&stale;t.mRenderGroup=7;}
 app.reanims[90]=&skin;auto* gatling=skinBoard.plant(0,0);gatling->mHeadReanimID=90;
 SandboxPlants::Assign(gatling,106);
 assert(instances[0].mImageOverride->path.ends_with("fire-gatling-head.png"));
 for(int i=1;i<=6;++i){assert(instances[i].mImageOverride==nullptr);assert(instances[i].mRenderGroup==7);}
 assert(instances[7].mImageOverride==&stale&&instances[8].mImageOverride==&stale);
 gatling->mBlinkCountdown=4;SandboxPlants::Tick(&skinBoard);
 assert(instances[0].mImageOverride->path.ends_with("fire-gatling-blink.png"));
 SandboxPlants::Reset();app.reanims.clear();
 Sexy::Graphics g(nullptr);g.mTransX=170;g.mTransY=250;
 SandboxArt::Sprite(&g,"fire",12,12,30,22);near(testBlits.back().matrix.m02,182);near(testBlits.back().matrix.m12,262);
 Reanimation a;a.track="idle_mouth";a.matrix={0,-0.72f,60,0.72f,0,40};float x,y;
 assert(SandboxArt::TrackPoint(&a,"idle_mouth",35,49,32,24.5f,x,y));near(x,60);near(y,50.44f);
 assert(!SandboxArt::TrackPoint(&a,"missing",35,49,32,24.5f,x,y));a.pose.mFrame=-1;
 assert(!SandboxArt::TrackPoint(&a,"idle_mouth",35,49,32,24.5f,x,y));a.pose.mFrame=0;
 Board board;auto* pea=board.plant(2,2);SandboxPlants::Assign(pea,113);pea->mHeadReanimID=1;app.reanims[1]=&a;
 auto* shot=board.AddProjectile(0,0,0,2,PROJECTILE_PEA);SandboxPlants::OnFired(pea,shot,nullptr);
 near(shot->mPosX+12,pea->mX+60);near(shot->mPosY+12,pea->mY+50.44f);near(SandboxPlants::ShotScale(shot),0.55f);assert(SandboxPlants::ShotRadius(shot)==5);
 g.mTransX=shot->mX;g.mTransY=shot->mY;testBlits.clear();assert(SandboxPlants::DrawShot(&g,shot));
 assert(testBlits.size()==1&&testBlits[0].path.ends_with("vfx-tiny.png"));near(testBlits[0].matrix.m02,shot->mPosX+12);near(testBlits[0].matrix.m12,shot->mPosY+12);
 auto* three=board.plant(3,2);three->mSeedType=static_cast<SeedType>(18);SandboxPlants::Assign(three,103);
 Reanimation heads[3];for(int i=0;i<3;++i){heads[i].track="ThreePeater_mouth"+std::to_string(i+1);heads[i].matrix.m02=40;heads[i].matrix.m12=70-i*23;app.reanims[i+2]=&heads[i];}
 three->mHeadReanimID=2;three->mHeadReanimID2=3;three->mHeadReanimID3=4;
 float ys[3];for(int row=1;row<=3;++row){auto* s=board.AddProjectile(0,0,0,row,PROJECTILE_PEA);s->mProjectileType=PROJECTILE_SNOWPEA;SandboxPlants::OnFired(three,s,nullptr);ys[row-1]=s->mPosY;assert(SandboxPlants::HasShot(s));}
 assert(ys[0]<ys[1]&&ys[1]<ys[2]);near(ys[1]-ys[0],23);near(ys[2]-ys[1],23);
 // Gatling projectiles leave the front barrel, never from behind its front plate.
 Reanimation barrel;barrel.track="GatlingPea_barrel3";barrel.matrix.m00=0.55f;barrel.matrix.m11=0.55f;barrel.matrix.m02=70.125f;barrel.matrix.m12=27.525f;app.reanims[5]=&barrel;
 auto* gun=board.plant(2,1);SandboxPlants::Assign(gun,105);gun->mHeadReanimID=5;
 auto* gunShot=board.AddProjectile(0,0,0,1,PROJECTILE_PEA);SandboxPlants::OnFired(gun,gunShot,nullptr);
 near(gunShot->mPosX+12,gun->mX+80.30f);near(gunShot->mPosY+12,gun->mY+27.525f);
 // All eight original variants get custom art without overriding native splash/slow damage.
 for(int id=100;id<=107;++id){auto* p=board.plant(1,1);SandboxPlants::Assign(p,id);auto* s=board.AddProjectile(40,40,0,1,PROJECTILE_PEA);SandboxPlants::OnFired(p,s,nullptr);assert(SandboxPlants::HasShot(s));assert(!SandboxPlants::Impact(s,nullptr));}
 SandboxPlants::Reset();app.reanims.clear();testBlits.clear();auto* acid=board.plant(1,2);SandboxPlants::Assign(acid,117);auto* s=board.AddProjectile(200,230,0,2,PROJECTILE_PEA);SandboxPlants::OnFired(acid,s,nullptr);
 auto* enemy=board.AddZombieInRow(ZOMBIE_NORMAL,2,0);SandboxPlants::Impact(s,enemy);SandboxPlants::DrawEffects(&g,&board,1);assert(testBlits.empty());
 SandboxPlants::DrawEffects(&g,&board,2);assert(!testBlits.empty());bool found=false;for(auto& b:testBlits)if(b.path.ends_with("vfx-acid-hit-0.png"))found=true;assert(found);
 const auto before=testBlits;board.mPaused=true;for(int i=0;i<100;++i)SandboxPlants::Tick(&board);testBlits.clear();SandboxPlants::DrawEffects(&g,&board,2);assert(before.size()==testBlits.size());for(size_t i=0;i<before.size();++i){assert(before[i].path==testBlits[i].path);near(before[i].matrix.m02,testBlits[i].matrix.m02);assert(before[i].alpha==testBlits[i].alpha);}board.mPaused=false;
 // Reverse homing must rotate the sprite AND its off-center tail around the pea core.
 auto* seeker=board.plant(1,2);SandboxPlants::Assign(seeker,116);auto* reverse=board.AddProjectile(200,230,0,2,PROJECTILE_PEA);SandboxPlants::OnFired(seeker,reverse,enemy);reverse->mVelX=-3;reverse->mVelY=0;reverse->mX=200;reverse->mY=230;g.mTransX=200;g.mTransY=230;testBlits.clear();SandboxPlants::DrawShot(&g,reverse);assert(testBlits.back().matrix.m00<0);near(testBlits.back().matrix.m02,216.5f);
 SandboxPlants::Reset();testBlits.clear();SandboxPlants::DrawEffects(&g,&board,2);assert(testBlits.empty());
 for(int id=112;id<=117;++id){const auto& art=SandboxVisualRules::Shots[SandboxVisualRules::ArtIndex(id,false,false)];assert(art.w<=30&&art.h<=24&&art.coreX>=0&&art.coreX<=art.w);}
 std::cout<<"Visual coordinate contracts passed: matrix pivot, scale, 3 independent mouths, projectile core, all 8 original variants, row effects and reset\n";
}
