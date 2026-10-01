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
 // Energy recolouring must not detach a head or leak into ordinary previews.
 {Reanimation body;Track tracks[]={{"DoomShroom_head1"},{"other"}};TrackInstance instances[2];body.def.mTracks={2,tracks};body.mTrackInstances=instances;
  auto* source=dynamic_cast<Sexy::MemoryImage*>(SandboxArt::NativeImage("DoomShroom_head1.png"));source->bits[0]=0xff151515;source->bits[1]=0x00808080;
  ReanimatorTransform original;original.mImage=source;original.mTransX=13;original.mTransY=-7;original.mSkewX=11;original.mScaleX=.8f;
  {AbstractRigVisuals::Scope scope(&body,MemeCharacters::NukeShroom);auto head=original;AbstractRigVisuals::Transform(&body,0,head);
   assert(head.mImage!=source);near(head.mTransX,13);near(head.mTransY,-7);near(head.mSkewX,11);near(head.mScaleX,.8f);
   auto* tinted=dynamic_cast<Sexy::MemoryImage*>(head.mImage);assert(tinted&&tinted->bits[0]==source->bits[0]&&tinted->bits[1]==source->bits[1]);
   auto other=original;AbstractRigVisuals::Transform(&body,1,other);assert(other.mImage==source);
  }
  auto normal=original;AbstractRigVisuals::Transform(&body,0,normal);assert(normal.mImage==source);
 }
 {Board b;SandboxPlants::Reset();Reanimation body;Track tracks[]={{"anim_face"},{"PuffShroom_tip"},{"PuffShroom_head"}};TrackInstance instances[3];body.def.mTracks={3,tracks};body.mTrackInstances=instances;app.reanims[91]=&body;
  for(int slot=0;slot<5;++slot){auto* p=b.plant(1,2);p->mSeedType=SeedType(8);p->mBodyReanimID=91;SandboxPlants::Assign(p,525);
   const auto pose=TinyPuffRules::At(slot);ReanimatorTransform original;original.mTransX=40;original.mTransY=65;
   {AbstractRigVisuals::Scope scoped(p);auto tip=original;AbstractRigVisuals::Transform(&body,1,tip);near(tip.mTransX,40);near(tip.mTransY,65);near(tip.mSkewX,pose.lean*180/3.14159265f);
    auto cap=original;AbstractRigVisuals::Transform(&body,2,cap);near(cap.mSkewX,(pose.lean+pose.cap)*180/3.14159265f);near(cap.mScaleX,1);near(cap.mScaleY,1);
   }
   auto native=original;AbstractRigVisuals::Transform(&body,1,native);near(native.mSkewX,0);near(native.mTransX,40);near(native.mTransY,65);
  }app.reanims.erase(91);SandboxPlants::Reset();
 }
 // Recolour only native orange, across all damage images. Do not turn
 // white bands, black outlines or transparent padding green.
 for(int stage=0;stage<3;++stage){const char* files[]={"Zombie_cone1.png","Zombie_cone2.png","Zombie_cone3.png"};auto* source=dynamic_cast<Sexy::MemoryImage*>(SandboxArt::NativeImage(files[stage]));assert(source);
  source->bits[0]=0xffff8000;source->bits[1]=0xff101010;source->bits[2]=0xffffffff;source->bits[3]=0x00ee8822;
  auto* coloured=dynamic_cast<Sexy::MemoryImage*>(SandboxArt::GreenCone(stage));assert(coloured&&coloured->mWidth==source->mWidth&&coloured->mHeight==source->mHeight);
  const auto p=coloured->bits[0];assert(((p>>8)&255)>((p>>16)&255)&&((p>>8)&255)>(p&255)&&(p>>24)==255);
  for(int i:{1,2,3})assert(coloured->bits[i]==source->bits[i]);
 }
 // Only the cone bone changes. Its bottom attachment is stable as layers
 // break; previews/rotation and native frame/render-group gates agree.
 for(int id:{218,219}){SandboxZombies::Reset();Board w;auto* z=w.AddZombieInRow(static_cast<ZombieType>(2),2,-1);SandboxZombies::Assign(z,id);
  Reanimation body;Track tracks[]={{"anim_cone"},{"anim_head1"},{"Zombie_body"}};TrackInstance instances[3];body.def.mTracks={3,tracks};body.mTrackInstances=instances;
  auto* original=SandboxArt::NativeImage("Zombie_cone1.png");for(auto& i:instances)i.mImageOverride=original;body.pose.mImage=original;
  app.reanims[96]=&body;z->mBodyReanimID=96;
  for(int armor:{7400,7399,7030,7029,370,249,123,1,0})for(float angle:{0.f,25.f,110.f}){
   z->mHelmHealth=id==218?std::min(armor,740):armor;
   body.pose.mTransX=13.1f;body.pose.mTransY=-18.7f;body.pose.mSkewX=body.pose.mSkewY=angle;body.pose.mScaleX=body.pose.mScaleY=.8f;
   {AbstractRigVisuals::Scope scope(z);auto hat=body.pose;AbstractRigVisuals::Transform(&body,0,hat);assert(instances[0].mImageOverride==nullptr);
    if(!armor)assert(hat.mAlpha==0);
    else{const int count=SandboxZombies::TowerCount(z->mHelmHealth);const float rise=id==219?6*(count-1):0,k=angle*3.14159265f/180;
     assert(hat.mImage==(id==218?SandboxArt::GreenCone(SandboxZombies::ConeDamageStage(z->mHelmHealth/2)):SandboxArt::ConeTower(z->mHelmHealth)));
     near(hat.mTransX-rise*.8f*std::sin(k),body.pose.mTransX);near(hat.mTransY+rise*.8f*std::cos(k),body.pose.mTransY);
     if(id==219)assert(hat.mImage->mHeight==original->mHeight+rise);
    }
    auto hidden=body.pose;hidden.mFrame=-1;AbstractRigVisuals::Transform(&body,0,hidden);assert(hidden.mImage==original&&hidden.mFrame==-1);
    for(int i:{1,2}){auto t=body.pose;AbstractRigVisuals::Transform(&body,i,t);assert(t.mImage==original&&instances[i].mImageOverride==original);near(t.mTransY,body.pose.mTransY);}
   }
   for(auto& i:instances)assert(i.mImageOverride==original&&i.mRenderGroup==0);
  }
  SandboxZombies::Forget(z);{AbstractRigVisuals::Scope scope(z);auto hat=body.pose;AbstractRigVisuals::Transform(&body,0,hat);assert(hat.mImage==original);}
  app.reanims.clear();
 }
 // The entire native rig rotates on exactly the same slowed clock as its
 // flight/lane curve. Native facing is mirrored once; the pivot stays fixed.
 for(int id:{216,217})for(float facing:{1.f,-1.f}){SandboxZombies::Reset();Board w;auto* z=w.AddZombieInRow(static_cast<ZombieType>(SandboxZombies::Base(id)),2,-1);SandboxZombies::Assign(z,id);
  z->mZombiePhase=SandboxZombies::CleverFlip;z->mBossMode=SandboxZombies::SlowFlipTag;
  for(int elapsed:{0,10,39,40,41,60,80,100,119,120,121,150,159,160}){
   z->mPhaseCounter=160-elapsed;Reanimation body;body.mOverlayMatrix={facing,0,220,0,1,150};
   SandboxZombies::AdjustPose(z,&body);const auto m=body.mOverlayMatrix;
   const float angle=SandboxZombies::SlowFlipProgress(float(elapsed)/160)*6.2831853f;
   near(m.m00,facing*std::cos(angle));near(m.m01,-facing*std::sin(angle));near(m.m10,std::sin(angle));near(m.m11,std::cos(angle));
   near(m.m02+m.m00*40+m.m01*80,220+facing*40);near(m.m12+m.m10*40+m.m11*80,230);
  }
  SandboxZombies::Forget(z);Reanimation ordinary;ordinary.mOverlayMatrix={facing,0,220,0,1,150};SandboxZombies::AdjustPose(z,&ordinary);near(ordinary.mOverlayMatrix.m00,facing);near(ordinary.mOverlayMatrix.m02,220);
 }
 // Both generated faces share one skull registration; body/hat/ring keep
 // native transforms and all temporary overrides are restored after drawing.
 for(int id:{216,217}){SandboxZombies::Reset();Board w;auto* z=w.AddZombieInRow(static_cast<ZombieType>(SandboxZombies::Base(id)),2,-1);SandboxZombies::Assign(z,id);
  Reanimation body;Track tracks[]={{"anim_head1"},{"anim_head2"},{"anim_tongue"},{"anim_hair"},{"anim_cone"},{"Zombie_duckytube"}};
  TrackInstance instances[6];body.def.mTracks={6,tracks};body.mTrackInstances=instances;
  auto* original=SandboxArt::NativeImage("Zombie_head.png");body.pose.mImage=original;
  for(auto& i:instances)i.mImageOverride=original;
  app.reanims[94]=&body;z->mBodyReanimID=94;
  auto* neutral=dynamic_cast<Sexy::MemoryImage*>(SandboxArt::CleverHead(false));auto* jaw=dynamic_cast<Sexy::MemoryImage*>(SandboxArt::CleverHead(true));
  assert(neutral&&jaw&&neutral!=jaw&&neutral->mWidth==jaw->mWidth&&neutral->mHeight==jaw->mHeight);
  int transparent=0,different=0;for(int i=0;i<neutral->mWidth*neutral->mHeight;++i){transparent+=(neutral->GetBits()[i]>>24)==0;different+=neutral->GetBits()[i]!=jaw->GetBits()[i];}
  assert(transparent>100&&different>50);
  for(int mode:{0,1,2})for(float angle:{0.f,90.f,210.f}){
   z->mZombiePhase=mode==1?SandboxZombies::CleverFlip:PHASE_ZOMBIE_NORMAL;z->mSummonCounter=mode==2?110:0;
   body.pose.mTransX=14;body.pose.mTransY=22;body.pose.mSkewX=body.pose.mSkewY=angle;body.pose.mScaleX=.8f;body.pose.mScaleY=.9f;
   {AbstractRigVisuals::Scope scope(z);auto head=body.pose;AbstractRigVisuals::Transform(&body,0,head);
    assert(head.mImage==(mode?jaw:neutral)&&instances[0].mImageOverride==nullptr);
    const float a=angle*3.14159265f/180;
    near(head.mTransX,14-.8f*std::cos(a)+4.5f*std::sin(a));near(head.mTransY,22-.8f*std::sin(a)-4.5f*std::cos(a));
    near(head.mScaleX,.8f);near(head.mScaleY,.9f);
    for(int i=1;i<4;++i)assert(instances[i].mRenderGroup==RENDER_GROUP_HIDDEN);
    for(int i:{4,5}){auto part=body.pose;AbstractRigVisuals::Transform(&body,i,part);assert(part.mImage==original&&instances[i].mImageOverride==original);near(part.mTransX,14);near(part.mTransY,22);}
    auto absent=body.pose;absent.mFrame=-1;AbstractRigVisuals::Transform(&body,0,absent);assert(absent.mFrame==-1&&absent.mImage==original);
    auto hidden=body.pose;hidden.mAlpha=0;AbstractRigVisuals::Transform(&body,0,hidden);assert(hidden.mAlpha==0&&hidden.mImage==original);
   }
   for(auto& i:instances)assert(i.mImageOverride==original&&i.mRenderGroup==RENDER_GROUP_NORMAL);
  }
  {AbstractRigVisuals::Scope preview(&body,id);auto head=body.pose;AbstractRigVisuals::Transform(&body,0,head);assert(head.mImage==neutral);}
  z->mHasHead=false;{AbstractRigVisuals::Scope dead(z);auto head=body.pose;AbstractRigVisuals::Transform(&body,0,head);assert(head.mImage==original);}
  SandboxZombies::Forget(z);z->mHasHead=true;{AbstractRigVisuals::Scope native(z);auto head=body.pose;AbstractRigVisuals::Transform(&body,0,head);assert(head.mImage==original);}
  app.reanims.clear();
 }
 // Opaque wrist pixels sit INSIDE the opening, on top of its dark fill.
 // Original green connector/rim stay present throughout idle/rise/recoil.
 {SandboxPlants::Reset();Board w;auto* plant=w.plant(2,2);plant->mSeedType=static_cast<SeedType>(26);SandboxPlants::Assign(plant,524);
  Reanimation body;Track tracks[]={{"Cactus_mouth"},{"Cactus_lips"},{"anim_face"}};TrackInstance instances[3];body.def.mTracks={3,tracks};body.mTrackInstances=instances;
  body.track="Cactus_lips";app.reanims[92]=&body;plant->mBodyReanimID=92;
  auto* palmImage=dynamic_cast<Sexy::MemoryImage*>(SandboxArt::Palm());assert(palmImage&&(palmImage->GetBits()[29*64+6]>>24)>240);
  for(float y:{19.5f,-54.5f,25.f})for(float angle:{0.f,10.f,-12.f})for(float offset:{0.f,224.f}){
   const float a=angle*3.14159265f/180;
   body.matrix={.8f*std::cos(a),-.84f*std::sin(a),50.4f+offset,.8f*std::sin(a),.84f*std::cos(a),y};
   Sexy::SexyTransform2D palm;assert(SandboxArt::PalmMatrix(&body,palm));
   const float mouthX=body.matrix.m02+body.matrix.m00*(5-17*.5f),mouthY=body.matrix.m12+body.matrix.m10*(5-17*.5f);
   near(palm.m02+palm.m00*(6-32)+palm.m01*(29-22),mouthX);
   near(palm.m12+palm.m10*(6-32)+palm.m11*(29-22),mouthY);
   near(std::hypot(palm.m00,palm.m10)*64,32);near(std::hypot(palm.m01,palm.m11)*44,.84f*27.5f);
   {AbstractRigVisuals::Scope scope(plant);for(int track=0;track<3;++track){auto part=body.pose;
    AbstractRigVisuals::Transform(&body,track,part);assert(part.mImage==body.pose.mImage);near(part.mTransX,body.pose.mTransX);near(part.mScaleX,body.pose.mScaleX);
   }}
   Sexy::Graphics g(nullptr);g.mTransX=71;g.mTransY=93;testBlits.clear();assert(SandboxPlants::DrawBody(&g,plant,0,0));assert(testBlits.size()==1);
   near(testBlits.back().matrix.m02,palm.m02+71);near(testBlits.back().matrix.m12,palm.m12+93);
   auto* emitted=w.AddProjectile(0,0,0,2,PROJECTILE_SPIKE);emitted->mPosZ=-10;MemeCharacters::OnFired(plant,emitted);
   near(emitted->mPosX+12,plant->mX+palm.m02);near(emitted->mPosY+emitted->mPosZ+12,plant->mY+palm.m12);
  }
  Sexy::SexyTransform2D hidden;body.pose.mFrame=-1;assert(!SandboxArt::PalmMatrix(&body,hidden));body.pose.mFrame=0;
  auto* shot=w.AddProjectile(352.7f,221.3f,0,2,PROJECTILE_SPIKE);shot->mX=352;shot->mY=221;
  for(int style:{299,300}){
   assert(MemeCharacters::RestoreShotStyle(shot,style));Sexy::Graphics g(nullptr);g.mTransX=576;g.mTransY=221;
   testBlits.clear();assert(SandboxPlants::DrawShot(&g,shot));assert(testBlits.size()==1);
   // Sprite origin was translated by the game already; no double world offset.
   near(testBlits.back().matrix.m02,588.7f);near(testBlits.back().matrix.m12,233.3f);
  }
  app.reanims.clear();
 }
 // Giant head/jaw share the imp's neck. All transforms and native image
 // overrides remain local to the new identity, including previews/death.
 {SandboxZombies::Reset();Board w;auto* z=w.AddZombieInRow(ZOMBIE_IMP,1,-1);SandboxZombies::Assign(z,215);
  Reanimation body;Track tracks[]={{"anim_head1"},{"anim_head2"},{"Zombie_imp_body1"}};TrackInstance instances[3];body.def.mTracks={3,tracks};body.mTrackInstances=instances;
  auto* original=SandboxArt::NativeImage("Zombie_imp_head.png");for(auto& i:instances)i.mImageOverride=original;
  body.pose.mTransX=34.3f;body.pose.mTransY=41.9f;body.pose.mSkewX=body.pose.mSkewY=12.6f;body.pose.mScaleX=body.pose.mScaleY=.999f;
  app.reanims[91]=&body;z->mBodyReanimID=91;
  for(int counter:{0,90,75,56,55,45,36,35,20,1}){
   z->mZombiePhase=counter?SandboxZombies::JawSmash:PHASE_ZOMBIE_NORMAL;z->mPhaseCounter=counter;
   {AbstractRigVisuals::Scope scope(z);auto head=body.pose,jaw=body.pose,torso=body.pose;
    AbstractRigVisuals::Transform(&body,0,head);AbstractRigVisuals::Transform(&body,1,jaw);AbstractRigVisuals::Transform(&body,2,torso);
    assert(head.mImage==SandboxArt::NativeImage("Zombie_gargantuar_head.png")&&jaw.mImage==SandboxArt::NativeImage("Zombie_gargantuar_jaw.png"));
    assert(instances[0].mImageOverride==nullptr&&instances[1].mImageOverride==nullptr&&instances[2].mImageOverride==original);
    near(torso.mTransX,body.pose.mTransX);near(torso.mScaleX,body.pose.mScaleX);
    const float hk=head.mSkewX*3.14159265f/180,jk=jaw.mSkewX*3.14159265f/180;
    const float nk=body.pose.mSkewX*3.14159265f/180;
    near(head.mTransX+43*head.mScaleX*std::cos(hk)-64*head.mScaleY*std::sin(hk),body.pose.mTransX+24*body.pose.mScaleX*std::cos(nk)-34*body.pose.mScaleY*std::sin(nk));
    near(head.mTransY+43*head.mScaleX*std::sin(hk)+64*head.mScaleY*std::cos(hk),body.pose.mTransY+24*body.pose.mScaleX*std::sin(nk)+34*body.pose.mScaleY*std::cos(nk));
    // Hinge (34,4) on the jaw must stay at (42.3,58.1) on the head,
    // regardless of windup, strike, walking rotation or recovery.
    near(jaw.mTransX+34*jaw.mScaleX*std::cos(jk)-4*jaw.mScaleY*std::sin(jk),head.mTransX+42.3f*head.mScaleX*std::cos(hk)-58.1f*head.mScaleY*std::sin(hk));
    near(jaw.mTransY+34*jaw.mScaleX*std::sin(jk)+4*jaw.mScaleY*std::cos(jk),head.mTransY+42.3f*head.mScaleX*std::sin(hk)+58.1f*head.mScaleY*std::cos(hk));
    auto hidden=body.pose;hidden.mFrame=-1;AbstractRigVisuals::Transform(&body,0,hidden);assert(hidden.mFrame==-1&&hidden.mImage==body.pose.mImage);
   }
   for(auto& i:instances)assert(i.mImageOverride==original);
  }
  {AbstractRigVisuals::Scope preview(&body,215);auto head=body.pose;AbstractRigVisuals::Transform(&body,0,head);assert(head.mImage==SandboxArt::NativeImage("Zombie_gargantuar_head.png"));}
  SandboxZombies::Forget(z);{AbstractRigVisuals::Scope ordinary(z);auto head=body.pose;AbstractRigVisuals::Transform(&body,0,head);assert(head.mImage==body.pose.mImage);}
  app.reanims.clear();
 }
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
