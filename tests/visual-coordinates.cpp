// Runs actual production skin/projectile/effect draw code against a captured raster API.
// It checks coordinate contracts, not a live browser/GPU screenshot.
#include "Engine.h"
#include "../src/AbstractRigVisuals.h"
#include "SandboxArt.h"
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "SandboxVisualRules.h"
#include "SandboxMemeRules.h"
#include "MemeShooterRules.h"
#include <cassert>
#include <cmath>
#include <iostream>
LawnApp app;LawnApp* gLawnApp=&app;bool gSandboxEnabled=true;
void near(float a,float b){assert(std::abs(a-b)<0.01f);}
int main(){
 // Tucking retracts the native stalk, head and every petal as one group.
 // No replacement face, sweat overlays, drifting parts or accumulated pose.
 {SandboxPlants::Reset();Board w;auto* p=w.plant(2,2);p->mSeedType=static_cast<SeedType>(53);SandboxPlants::Assign(p,520);
  Reanimation body;Track tracks[]={{"anim_idle"},{"SunFlower_leftpetal1"},{"anim_blink"},{"stalk_top"},{"SunFlower_rightpetal9"},{"SunFlower_toppetals"},{"SunFlower_bottompetals"},{"frontleaf"},{"backleaf"},{"stalk_bottom"}};TrackInstance instances[10];body.def.mTracks={10,tracks};body.mTrackInstances=instances;body.track="anim_idle";body.matrix={.8f,0,35,0,.7f,40};app.reanims[82]=&body;p->mBodyReanimID=82;
  ReanimatorTransform original;original.mImage=SandboxArt::NativeImage("SunFlower_head.png");original.mTransX=14.3f;original.mTransY=20.4f;original.mScaleX=.8f;original.mScaleY=.712f;
  for(int age:{0,1,40,300,900}){
   assert(MemeCharacters::Restore(p,{520,300,1,0,0,0,age,0,0,3}));
   AbstractRigVisuals::Scope scope(p);auto face=original;AbstractRigVisuals::Transform(&body,0,face);assert(face.mImage==original.mImage);near(face.mSkewX,0);
   near(face.mTransX,37+(original.mTransX-37)*.8f);near(face.mTransY,45+(original.mTransY-45)*.8f+24);
   for(int track:{1,2,4,5,6}){auto part=original;AbstractRigVisuals::Transform(&body,track,part);
    near(part.mSkewX,face.mSkewX);near(part.mSkewY,face.mSkewY);near(part.mTransX,face.mTransX);near(part.mTransY,face.mTransY);
    near(part.mScaleX,original.mScaleX*.8f);near(part.mScaleY,original.mScaleY*.8f);assert(part.mImage==original.mImage);
   }
   for(int track:{3,9}){auto stalk=original;AbstractRigVisuals::Transform(&body,track,stalk);near(stalk.mTransX,original.mTransX);near(stalk.mTransY,80+(original.mTransY-80)*.35f);near(stalk.mScaleY,original.mScaleY*.35f);}
   for(int track:{7,8}){auto leaf=original;AbstractRigVisuals::Transform(&body,track,leaf);near(leaf.mTransX,original.mTransX);near(leaf.mTransY,original.mTransY);near(leaf.mScaleY,original.mScaleY);}
  }
  // Old saves lose embarrassment/gaze. Native pose is restored exactly.
  assert(MemeCharacters::Restore(p,{520,300,1,22,300,300,100,0,1,1}));assert(!MemeCharacters::Hiding(p));
  {AbstractRigVisuals::Scope scope(p);for(int track=0;track<10;++track){auto part=original;AbstractRigVisuals::Transform(&body,track,part);
   near(part.mTransX,original.mTransX);near(part.mTransY,original.mTransY);near(part.mSkewX,original.mSkewX);
   assert(part.mImage==original.mImage);near(part.mScaleY,original.mScaleY);
  }}
  auto unscoped=original;AbstractRigVisuals::Transform(&body,0,unscoped);assert(unscoped.mImage==original.mImage);
  Sexy::Graphics g(nullptr);g.mTransX=224;g.mTransY=50;
  for(int phase:{0,1,0,1,0}){
   assert(MemeCharacters::Restore(p,{520,300,phase,0,0,0,100,0,0,3}));
   testBlits.clear();assert(SandboxPlants::DrawBody(&g,p,0,0));assert(testBlits.empty());
  }
  assert(MemeCharacters::Restore(p,{520,300,0,0,0,0,500,0,0,3}));
  {AbstractRigVisuals::Scope scope(p);auto face=original;AbstractRigVisuals::Transform(&body,0,face);assert(face.mImage==original.mImage);near(face.mTransX,original.mTransX);}
  testBlits.clear();assert(SandboxPlants::DrawBody(&g,p,0,0));assert(testBlits.empty());app.reanims.clear();
 }
 // Automatic rage has only the existing bar: no manual-ready glow or notch.
 for(int heat:{80,100,280})for(int phase:{0,1})for(bool asleep:{false,true}){
  SandboxPlants::Reset();Board w;auto* p=w.plant(1,2);SandboxPlants::Assign(p,500);p->mIsAsleep=asleep;
  assert(SandboxPlants::RestorePower(p,{500,300,phase,phase?0:heat,0,50,40,0,phase?25:0,2}));
  Sexy::Graphics g(nullptr);Sexy::drawnRects.clear();MemeCharacters::Effects(&g,&w,2);
  assert(Sexy::drawnRects.size()==2);
  const auto& bar=Sexy::drawnRects[0];assert(bar.bounds.mX==p->mX+12&&bar.bounds.mY==p->mY+77&&bar.bounds.mWidth==56&&bar.bounds.mHeight==6);
 }
 // Native flowerpot lift and pool bobbing move indicators and steam together.
 for(int offset:{-5,-2,0,2})for(int phase:{0,1}){
  SandboxPlants::Reset();Board w;auto* p=w.plant(1,2);p->drawHeightOffset=offset;SandboxPlants::Assign(p,500);
  assert(SandboxPlants::RestorePower(p,{500,300,phase,phase?0:100,0,50,40,0,phase?25:0,2}));
  Sexy::Graphics g(nullptr);g.mTransX=224;g.mTransY=17;Sexy::drawnRects.clear();testBlits.clear();MemeCharacters::Effects(&g,&w,2);
  const auto& bar=Sexy::drawnRects[0];assert(bar.bounds.mY==p->mY+offset+77);
  if(phase){assert(testBlits.size()==1);near(testBlits[0].matrix.m02,p->mX+45+224);near(testBlits[0].matrix.m12,p->mY+offset+15-10+17);}
  p->airborne=true;Sexy::drawnRects.clear();testBlits.clear();MemeCharacters::Effects(&g,&w,2);
  assert(Sexy::drawnRects.empty()&&testBlits.empty());
 }
 // Bumping walnut never replaces the native face with a mouth or teeth,
 // including both original damage stages.
 for(int stage=0;stage<3;++stage){
  SandboxPlants::Reset();Board w;auto* p=w.plant(1,2);p->mSeedType=static_cast<SeedType>(3);
  p->mPlantMaxHealth=4000;p->mPlantHealth=stage==0?4000:stage==1?2000:800;SandboxPlants::Assign(p,501);
  p->mRecentlyEatenCountdown=50;auto* z=w.AddZombieInRow(ZOMBIE_NORMAL,2,-1);z->mPosX=p->mX-30;z->mIsEating=true;
  Reanimation body;body.track="anim_face";body.matrix.m02=120;body.matrix.m12=220;Track tracks[]={{"anim_face"}};TrackInstance instances[1];body.def.mTracks={1,tracks};body.mTrackInstances=instances;
  auto* original=SandboxArt::NativeImage(stage==0?"Wallnut_body.png":stage==1?"Wallnut_cracked1.png":"Wallnut_cracked2.png");instances[0].mImageOverride=original;app.reanims[80]=&body;p->mBodyReanimID=80;
  Sexy::Graphics g(nullptr);for(int tick=0;tick<60;++tick){SandboxPlants::Tick(&w);testBlits.clear();const bool angry=MemeCharacters::Data(p,0)==1;
   assert(SandboxPlants::DrawBody(&g,p,0,0));assert(instances[0].mImageOverride==original);
   assert(testBlits.size()==(angry?2:0));if(angry){near(testBlits[0].matrix.m02,114);near(testBlits[0].matrix.m12,199);near(testBlits[1].matrix.m02,143);near(testBlits[1].matrix.m12,196);}
  }
  app.reanims.clear();
 }
 SandboxPlants::Reset();
 Sexy::Graphics g(nullptr);Reanimation mouth;mouth.track="idle_mouth";mouth.matrix={0,-0.72f,60,0.72f,0,40};
 Board board;app.reanims[1]=&mouth;
 // New rage shots keep the exact native muzzle registration and pea scale.
 auto* rage=board.plant(1,2);SandboxPlants::Assign(rage,500);rage->mHeadReanimID=1;
 auto* wave=board.AddProjectile(0,0,0,2,PROJECTILE_PEA);SandboxPlants::OnFired(rage,wave,nullptr);
 near(wave->mPosX+12,rage->mX+60);near(wave->mPosY+12,rage->mY+50.44f);near(SandboxPlants::ShotScale(wave),1);
 assert(MemeCharacters::ShotStyle(wave)>0&&!SandboxPlants::DrawShot(&g,wave));
 {Reanimation body;Track tracks[]={{"anim_face"}};TrackInstance instances[1];body.def.mTracks={1,tracks};body.mTrackInstances=instances;
  auto* native=SandboxArt::NativeImage("PeaShooter_Head.png");instances[0].mImageOverride=native;app.reanims[80]=&body;rage->mBodyReanimID=80;
  assert(SandboxPlants::RestorePower(rage,{500,300,1,0,0,0,0,0,50,2}));
  assert(SandboxPlants::DrawBody(&g,rage,0,0)&&instances[0].mImageOverride==native);
  assert(SandboxPlants::RestorePower(rage,{500,300,0,0,0,0,0,0,0,2}));
  assert(SandboxPlants::DrawBody(&g,rage,0,0)&&instances[0].mImageOverride==native);
  for(int heat:{20,60,100,140,180,200,240,280,300}){assert(SandboxPlants::RestorePower(rage,{500,300,0,heat,0,0,0,0,0,2}));drawnOverrides.clear();
   assert(SandboxPlants::DrawBody(&g,rage,0,0));assert(drawnOverrides[0]==SandboxArt::WarmNative("PeaShooter_Head.png",heat*24/MemeShooterRules::MaxRage));assert(instances[0].mImageOverride==native);
  }app.reanims.erase(80);
 }

 {auto* p=board.plant(3,2);p->mSeedType=static_cast<SeedType>(52);SandboxPlants::Assign(p,519);
  Reanimation head;Track tracks[]={{"anim_face"},{"idle_mouth"},{"idle_headleaf_nearest"},{"anim_blink"},{"stalk_top"}};TrackInstance instances[5];head.def.mTracks={5,tracks};head.mTrackInstances=instances;
  app.reanims[81]=&head;p->mHeadReanimID=81;Sexy::Image pea;pea.mWidth=pea.mHeight=27;Sexy::IMAGE_PROJECTILEPEA=&pea;
  ReanimatorTransform original;original.mTransX=19.2f;original.mTransY=17.8f;original.mScaleX=.555f;original.mScaleY=.5f;
  {AbstractRigVisuals::Scope scope(p);auto t=original;AbstractRigVisuals::Transform(&head,0,t);
   assert(t.mImage==&pea);near(t.mTransX+13.5f*t.mScaleX,19.2f+35*.555f);near(t.mTransY+13.5f*t.mScaleY,17.8f+32.5f*.5f);
   near(t.mScaleX*27,84*.555f);near(t.mScaleY*27,93*.5f);
   for(int track:{1,2,3}){auto hidden=original;AbstractRigVisuals::Transform(&head,track,hidden);assert(hidden.mAlpha==0);}
   auto stem=original;AbstractRigVisuals::Transform(&head,4,stem);assert(stem.mAlpha==1&&stem.mScaleX==original.mScaleX);
  }
  auto native=original;AbstractRigVisuals::Transform(&head,0,native);assert(native.mImage==original.mImage&&native.mScaleX==original.mScaleX);
  {AbstractRigVisuals::Scope scope(&head,519);auto t=original;AbstractRigVisuals::Transform(&head,0,t);assert(t.mImage==&pea);}
  auto* projectile=board.AddProjectile(300,240,0,2,PROJECTILE_PEA);projectile->mMotionType=MOTION_STAR;assert(MemeCharacters::RestoreShotStyle(projectile,296));
  projectile->mX=300;projectile->mY=240;testBlits.clear();g.mTransX=331;g.mTransY=257;assert(SandboxPlants::DrawShot(&g,projectile));assert(testBlits.size()==1);
  near(testBlits[0].matrix.m02,343);near(testBlits[0].matrix.m12,266.2f);near(testBlits[0].matrix.m00,.56f);
  projectile->mProjectileAge=20;testBlits.clear();assert(SandboxPlants::DrawShot(&g,projectile));assert(std::abs(testBlits[0].matrix.m10)>.5f);
  Sexy::IMAGE_PROJECTILEPEA=nullptr;app.reanims.erase(81);
 }
 for(int id=502;id<=518;++id){auto* p=board.plant(1,1);p->mSeedType=static_cast<SeedType>(MemeCharacters::RetiredBase(id));SandboxPlants::Assign(p,id);
  float x=17,y=23,sx=.8f,sy=1.2f;SandboxPlants::AdjustScale(p,x,y,sx,sy);
  near(x,17);near(y,23);near(sx,.8f);near(sy,1.2f);assert(!SandboxPlants::DrawBody(&g,p,0,0));
 }
 std::cout<<"Visual coordinate contracts passed\\n";
}
