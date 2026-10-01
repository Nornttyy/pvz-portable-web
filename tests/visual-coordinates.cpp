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
#include "ConeBodyRules.h"
#include <cassert>
#include <cmath>
#include <iostream>
LawnApp app;LawnApp* gLawnApp=&app;bool gSandboxEnabled=true;
void near(float a,float b){assert(std::abs(a-b)<0.01f);}
int main(){
 // Exactly twenty cones, never flesh/clothing/duck art, even at zero armor.
 // Hidden hair/tongue bones are visible only inside this character's scope.
 {SandboxZombies::Reset();Board w;auto* z=w.AddZombieInRow(static_cast<ZombieType>(2),2,-1);SandboxZombies::Assign(z,214);
  static_assert(ConeBodyRules::Parts.size()==20&&SandboxZombies::ConeVisualCount==20);
  for(const auto& fit:ConeBodyRules::Parts){assert(fit.width>=34&&fit.height>=33);assert(std::abs(fit.width/fit.height-59.f/57)<.025f);}
  // Enlarging the feet must not push their bases below the old ground anchors.
  near(ConeBodyRules::Parts[16].cy+ConeBodyRules::Parts[16].height*.5f,19.5f);
  near(ConeBodyRules::Parts[19].cy+ConeBodyRules::Parts[19].height*.5f,24.f);
  Reanimation body;Track tracks[23];for(int i=0;i<20;++i)tracks[i].mName=ConeBodyRules::Parts[i].track.data();
  tracks[20].mName="Zombie_duckytube";tracks[21].mName="Zombie_mustache";tracks[22].mName="anim_bucket";
  TrackInstance instances[23];body.def.mTracks={23,tracks};body.mTrackInstances=instances;app.reanims[90]=&body;z->mBodyReanimID=90;
  auto* originalImage=SandboxArt::NativeImage("Zombie_body.png");for(auto& i:instances){i.mImageOverride=originalImage;i.mRenderGroup=RENDER_GROUP_HIDDEN;}
  const char* files[]={"Zombie_cone1.png","Zombie_cone2.png","Zombie_cone3.png"};
  for(int health:{2590,2350,2200,1450,330,1,0}){
   z->mHelmHealth=health;
   {AbstractRigVisuals::Scope scope(z);
    int visible=0;
    for(int part=0;part<23;++part){
     ReanimatorTransform t;t.mImage=originalImage;t.mTransX=14;t.mTransY=21;t.mScaleX=.8f;t.mScaleY=.7f;t.mSkewX=t.mSkewY=25;
     const auto before=t;AbstractRigVisuals::Transform(&body,part,t);
     if(part<20){
      ++visible;assert(t.mAlpha==1&&instances[part].mImageOverride==nullptr&&instances[part].mRenderGroup==RENDER_GROUP_NORMAL);
      assert(t.mImage==SandboxArt::NativeImage(files[SandboxZombies::ConeDamageStage(SandboxZombies::ConeVisualHealth(health,part))]));
      const auto& fit=ConeBodyRules::Parts[part];
      near(t.mScaleX*t.mImage->mWidth,before.mScaleX*fit.width);near(t.mScaleY*t.mImage->mHeight,before.mScaleY*fit.height);
      const float k=25*3.14159265f/180;
      near(t.mTransX+t.mImage->mWidth*.5f*t.mScaleX*std::cos(k)-t.mImage->mHeight*.5f*t.mScaleY*std::sin(k),before.mTransX+fit.cx*before.mScaleX*std::cos(k)-fit.cy*before.mScaleY*std::sin(k));
      near(t.mTransY+t.mImage->mWidth*.5f*t.mScaleX*std::sin(k)+t.mImage->mHeight*.5f*t.mScaleY*std::cos(k),before.mTransY+fit.cx*before.mScaleX*std::sin(k)+fit.cy*before.mScaleY*std::cos(k));
      near(t.mSkewX,25);near(t.mSkewY,25);assert(std::isfinite(t.mTransX)&&std::isfinite(t.mTransY)&&t.mScaleX>0&&t.mScaleY>0);
      auto moved=before;moved.mTransX+=147;moved.mTransY-=83;AbstractRigVisuals::Transform(&body,part,moved);near(moved.mTransX-t.mTransX,147);near(moved.mTransY-t.mTransY,-83);
     }else{assert(t.mAlpha==0&&instances[part].mRenderGroup==RENDER_GROUP_HIDDEN);}
    }
    assert(visible==20);
    ReanimatorTransform absent;absent.mFrame=-1;absent.mImage=originalImage;AbstractRigVisuals::Transform(&body,0,absent);assert(absent.mFrame==-1&&absent.mImage==originalImage);
    ReanimatorTransform chewing;chewing.mFrame=-1;chewing.mImage=originalImage;AbstractRigVisuals::Transform(&body,4,chewing);assert(chewing.mFrame==0&&chewing.mImage!=originalImage);
   }
   assert(z->mHelmHealth==health);for(auto& i:instances)assert(i.mImageOverride==originalImage&&i.mRenderGroup==RENDER_GROUP_HIDDEN);
  }
  {AbstractRigVisuals::Scope preview(&body,214);ReanimatorTransform t;t.mImage=originalImage;AbstractRigVisuals::Transform(&body,6,t);assert(t.mImage==SandboxArt::NativeImage(files[0]));}
  SandboxZombies::Forget(z);{AbstractRigVisuals::Scope ordinary(z);ReanimatorTransform t;t.mImage=originalImage;AbstractRigVisuals::Transform(&body,1,t);assert(t.mImage==originalImage);}
  app.reanims.clear();
 }
 // Tucking retracts the native stalk, head and every petal as one group.
 // No replacement face, sweat overlays, drifting parts or accumulated pose.
 {SandboxPlants::Reset();Board w;auto* p=w.plant(2,2);p->mSeedType=SEED_SUNFLOWER;SandboxPlants::Assign(p,520);
  Reanimation body;Track tracks[]={{"anim_idle"},{"SunFlower_leftpetal1"},{"anim_blink"},{"stalk_top"},{"SunFlower_rightpetal9"},{"SunFlower_toppetals"},{"SunFlower_bottompetals"},{"frontleaf"},{"backleaf"},{"stalk_bottom"}};TrackInstance instances[10];body.def.mTracks={10,tracks};body.mTrackInstances=instances;body.track="anim_idle";body.matrix={.8f,0,35,0,.7f,40};app.reanims[82]=&body;p->mBodyReanimID=82;
  ReanimatorTransform original;original.mImage=SandboxArt::NativeImage("SunFlower_head.png");original.mTransX=14.3f;original.mTransY=20.4f;original.mScaleX=.8f;original.mScaleY=.712f;
  {AbstractRigVisuals::Scope preview(&body,520);for(int track=0;track<10;++track){auto part=original;AbstractRigVisuals::Transform(&body,track,part);near(part.mTransX,original.mTransX);near(part.mTransY,original.mTransY);near(part.mScaleY,original.mScaleY);}}
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
 // Rage never draws a heat/progress bar, regardless of phase or sleep.
 for(int heat:{80,100,280})for(int phase:{0,1})for(bool asleep:{false,true}){
  SandboxPlants::Reset();Board w;auto* p=w.plant(1,2);SandboxPlants::Assign(p,500);p->mIsAsleep=asleep;
  assert(SandboxPlants::RestorePower(p,{500,300,phase,phase?0:heat,0,50,40,0,phase?25:0,2}));
  Sexy::Graphics g(nullptr);Sexy::drawnRects.clear();MemeCharacters::Effects(&g,&w,2);
  assert(Sexy::drawnRects.empty());
 }
 // Native flowerpot lift and pool bobbing still move steam, without a bar.
 for(int offset:{-5,-2,0,2})for(int phase:{0,1}){
  SandboxPlants::Reset();Board w;auto* p=w.plant(1,2);p->drawHeightOffset=offset;SandboxPlants::Assign(p,500);
  assert(SandboxPlants::RestorePower(p,{500,300,phase,phase?0:100,0,50,40,0,phase?25:0,2}));
  Sexy::Graphics g(nullptr);g.mTransX=224;g.mTransY=17;Sexy::drawnRects.clear();testBlits.clear();MemeCharacters::Effects(&g,&w,2);
  assert(Sexy::drawnRects.empty());
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
 // Gatling keeps native steam but no heat or cooldown progress bar.
 {Board w;auto* p=w.plant(2,2);p->mSeedType=static_cast<SeedType>(40);SandboxPlants::Assign(p,522);
  Sexy::Graphics g(nullptr);
  for(const auto state:{std::array<int,10>{522,300,0,60,0,10,50,0,0,1},std::array<int,10>{522,300,1,120,175,0,50,0,0,1}}){
   assert(SandboxPlants::RestorePower(p,state));Sexy::drawnRects.clear();testBlits.clear();MemeCharacters::Effects(&g,&w,2);
   assert(Sexy::drawnRects.empty());
   assert(testBlits.size()==(state[2]?1:0));
  }
  SandboxPlants::Forget(p);
 }
 // Gatling's head, mouth and blinks warm together; its helmet, barrels,
 // native pivots, preview art and other instances never inherit the tint.
 {SandboxPlants::Reset();Board w;auto* p=w.plant(2,2);p->mSeedType=static_cast<SeedType>(40);SandboxPlants::Assign(p,522);
  Reanimation body;Track tracks[]={{"anim_face"},{"GatlingPea_mouth"},{"GatlingPea_mouth_overlay"},{"idle_shoot_blink"},{"anim_blink"},{"GatlingPea_helmet"},{"GatlingPea_barrel1"}};TrackInstance instances[7];body.def.mTracks={7,tracks};body.mTrackInstances=instances;app.reanims[83]=&body;p->mBodyReanimID=83;
  const char* files[]={"GatlingPea_head.png","GatlingPea_mouth.png","GatlingPea_mouth_overlay.png","GatlingPea_blink1.png","GatlingPea_blink1.png","GatlingPea_helmet.png","GatlingPea_barrel.png"};
  for(int i=0;i<7;++i)instances[i].mImageOverride=SandboxArt::NativeImage(files[i]);
  Sexy::Graphics g(nullptr);
  for(int cooling:{0,1})for(int level=0;level<=24;++level){
   if(!cooling&&level==24)continue;
   const int timer=cooling?std::max(1,(level*350+23)/24):0;
   assert(SandboxPlants::RestorePower(p,{522,300,cooling,cooling?120:level*5,timer,cooling?0:10,50,0,0,1}));
   const auto before=SandboxPlants::SavePower(p);drawnOverrides.clear();assert(SandboxPlants::DrawBody(&g,p,0,0));
   assert(drawnOverrides.size()==7&&SandboxPlants::SavePower(p)==before);
   for(int i=0;i<7;++i){assert(drawnOverrides[i]==(i<5?SandboxArt::WarmNative(files[i],level):SandboxArt::NativeImage(files[i])));assert(instances[i].mImageOverride==SandboxArt::NativeImage(files[i]));}
  }
  body.mAnimTime=.75f;assert(SandboxPlants::RestorePower(p,{522,300,0,60,0,10,50,0,0,1}));drawnOverrides.clear();assert(SandboxPlants::DrawBody(&g,p,0,0));
  assert(drawnOverrides[3]==SandboxArt::WarmNative("GatlingPea_blink2.png",12)&&drawnOverrides[4]==drawnOverrides[3]);
  drawnOverrides.clear();body.Draw(&g);assert(drawnOverrides[0]==SandboxArt::NativeImage(files[0]));app.reanims.clear();SandboxPlants::Reset();
 }
 Sexy::Graphics g(nullptr);Reanimation mouth;mouth.track="idle_mouth";mouth.matrix={0,-0.72f,60,0.72f,0,40};
 Board board;app.reanims[1]=&mouth;
 // New rage shots keep the exact native muzzle registration and pea scale.
 auto* rage=board.plant(1,2);SandboxPlants::Assign(rage,500);rage->mHeadReanimID=1;
 auto* wave=board.AddProjectile(0,0,0,2,PROJECTILE_PEA);SandboxPlants::OnFired(rage,wave,nullptr);
 near(wave->mPosX+12,rage->mX+60);near(wave->mPosY+12,rage->mY+50.44f);near(SandboxPlants::ShotScale(wave),1);
 assert(MemeCharacters::ShotStyle(wave)>0&&!SandboxPlants::DrawShot(&g,wave));
 // Only the 50-shot Repeater's peas shrink. Native muzzle coordinates,
 // collision radius, damage tag and other shooters remain untouched.
 {auto* p=board.plant(2,2);p->mSeedType=static_cast<SeedType>(7);SandboxPlants::Assign(p,521);
  auto* shot=board.AddProjectile(220,250,0,2,PROJECTILE_PEA);SandboxPlants::OnFired(p,shot,nullptr);
  near(SandboxPlants::ShotScale(shot),.6f);near(shot->mPosX,220);near(shot->mPosY,250);
  assert(SandboxPlants::ShotRadius(shot)==12&&SandboxPlants::ShotDamage(shot,20)==1);
  assert(!SandboxPlants::DrawShot(&g,shot));
  const int saved=SandboxPlants::SaveShot(shot);SandboxPlants::ForgetShot(shot);SandboxPlants::RestoreShot(shot,saved);near(SandboxPlants::ShotScale(shot),.6f);
  shot->mProjectileType=PROJECTILE_SNOWPEA;near(SandboxPlants::ShotScale(shot),.6f);
  shot->mProjectileType=PROJECTILE_FIREBALL;near(SandboxPlants::ShotScale(shot),1);assert(SandboxPlants::ShotDamage(shot,40)==2);
  auto* native=board.AddProjectile(220,250,0,2,PROJECTILE_PEA);near(SandboxPlants::ShotScale(native),1);near(SandboxPlants::ShotScale(wave),1);
  // The native pea centre stays at 12px even when its raster cel is 27px.
  near(SandboxVisualRules::NativePeaOffset(27,.6f)+(12-13.5f)*.6f,12);
 }
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
