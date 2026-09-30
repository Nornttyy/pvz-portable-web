// Runs actual production skin/projectile/effect draw code against a captured raster API.
// It checks coordinate contracts, not a live browser/GPU screenshot.
#include "Engine.h"
#include "SandboxArt.h"
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "SandboxVisualRules.h"
#include "SandboxMemeRules.h"
#include <cassert>
#include <cmath>
#include <iostream>
LawnApp app;LawnApp* gLawnApp=&app;bool gSandboxEnabled=true;
void near(float a,float b){assert(std::abs(a-b)<0.01f);}
int main(){
 // A ready shooter adds only a one-pixel muted outline around the existing
 // bar. No indicator below 100, during release or while asleep.
 for(int heat:{80,100,280})for(int phase:{0,1})for(bool asleep:{false,true}){
  SandboxPlants::Reset();Board w;auto* p=w.plant(1,2);SandboxPlants::Assign(p,500);p->mIsAsleep=asleep;
  assert(SandboxPlants::RestorePower(p,{500,300,phase,phase?0:heat,0,50,40,0,phase?25:0,2}));
  Sexy::Graphics g(nullptr);Sexy::drawnRects.clear();MemeCharacters::Effects(&g,&w,2);
  const bool ready=heat>=100&&!phase&&!asleep;assert(Sexy::drawnRects.size()==(ready?7:3));
  if(ready){const auto& edge=Sexy::drawnRects[0];
   assert(edge.bounds.mX==p->mX+11&&edge.bounds.mY==p->mY+76&&edge.bounds.mWidth==58&&edge.bounds.mHeight==1);
   assert(edge.color.mRed==210&&edge.color.mGreen==164&&edge.color.mBlue==77&&edge.color.mAlpha>=112&&edge.color.mAlpha<=160);
  }
  const auto& bar=Sexy::drawnRects[ready?4:0];assert(bar.bounds.mX==p->mX+12&&bar.bounds.mY==p->mY+77&&bar.bounds.mWidth==56&&bar.bounds.mHeight==6);
 }
 // Native flowerpot lift and pool bobbing move indicators and steam together.
 for(int offset:{-5,-2,0,2})for(int phase:{0,1}){
  SandboxPlants::Reset();Board w;auto* p=w.plant(1,2);p->drawHeightOffset=offset;SandboxPlants::Assign(p,500);
  assert(SandboxPlants::RestorePower(p,{500,300,phase,phase?0:100,0,50,40,0,phase?25:0,2}));
  Sexy::Graphics g(nullptr);g.mTransX=224;g.mTransY=17;Sexy::drawnRects.clear();testBlits.clear();MemeCharacters::Effects(&g,&w,2);
  const auto& bar=Sexy::drawnRects[phase?0:4];assert(bar.bounds.mY==p->mY+offset+77);
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
   assert(SandboxPlants::DrawBody(&g,p,0,0)==angry);assert(instances[0].mImageOverride==original);
   assert(testBlits.size()==(angry?2:0));if(angry){near(testBlits[0].matrix.m02,114);near(testBlits[0].matrix.m12,199);near(testBlits[1].matrix.m02,143);near(testBlits[1].matrix.m12,196);}
  }
  app.reanims.clear();
 }
 SandboxPlants::Reset();
 // Runtime palettes retain native dimensions/alpha, are cached, and never
 // mutate base pixels or the persistent damage override used by AnimateNuts.
 {auto* source=dynamic_cast<Sexy::MemoryImage*>(SandboxArt::NativeImage("Wallnut_cracked1.png"));
  source->GetBits()[0]=0x80b88742;auto* heated=dynamic_cast<Sexy::MemoryImage*>(SandboxArt::WarmNative("Wallnut_cracked1.png",2));
  assert(source!=heated&&source->GetBits()[0]==0x80b88742&&(heated->GetBits()[0]>>24)==0x80);
  assert(heated==SandboxArt::WarmNative("Wallnut_cracked1.png",2)&&heated->mWidth==source->mWidth&&heated->mHeight==source->mHeight);
  Board w;auto* p=w.plant(1,1);p->mSeedType=static_cast<SeedType>(3);p->mPlantMaxHealth=4000;p->mPlantHealth=2000;SandboxPlants::Assign(p,122);
  Reanimation body;Track tracks[]={{"anim_face"}};TrackInstance instances[1];body.def.mTracks={1,tracks};body.mTrackInstances=instances;
  instances[0].mImageOverride=source;app.reanims[80]=&body;p->mBodyReanimID=80;Sexy::Graphics g(nullptr);drawnOverrides.clear();
  assert(SandboxPlants::DrawBody(&g,p,0,0));assert(drawnOverrides.back()==heated);assert(instances[0].mImageOverride==source);
  assert(SandboxPlants::KeepsNativeBlink(p));SandboxPlants::Reset();app.reanims.clear();}

 for(int power:{180,181,182}){
  auto* source=dynamic_cast<Sexy::MemoryImage*>(SandboxArt::NativeImage("PeaShooter_Head.png"));source->GetBits()[0]=0x8066bc22;
  auto* palette=dynamic_cast<Sexy::MemoryImage*>(SandboxArt::PowerNative("PeaShooter_Head.png",24,power));
  assert(palette!=source&&source->GetBits()[0]==0x8066bc22&&(palette->GetBits()[0]>>24)==0x80);
 }
 // Every supported native face and mouth uses its own canvas, including snow,
 // threepeater and gatling parts. Applying a power never leaves shared overrides.
 struct SkinCase {int base;const char* face;const char* image;const char* mouth;const char* mouthImage;};
 const SkinCase skins[]={{0,"anim_face","PeaShooter_Head.png","idle_mouth","PeaShooter_mouth.png"},
  {7,"anim_face","PeaShooter_Head.png","idle_mouth","PeaShooter_mouth.png"},
  {5,"anim_face","SnowPea_head.png","SnowPea_mouth","SnowPea_mouth.png"},
  {18,"anim_face1","ThreePeater_head.png","ThreePeater_mouth1","ThreePeater_mouth.png"},
  {40,"GatlingPea_head","GatlingPea_head.png","GatlingPea_barrel3",nullptr},
  {1,"anim_idle","SunFlower_head.png","leaf",nullptr}};
 for(int power:{180,181,182})for(const auto& entry:skins){
  SandboxPlants::Reset();Board w;auto* p=w.plant(1,1);p->mSeedType=static_cast<SeedType>(entry.base);
  SandboxPlants::Assign(p,SandboxMemeRules::Result(entry.base,power));
  Reanimation anim;Track tracks[]={{entry.face},{entry.mouth},{"native_stem"}};TrackInstance instances[3];
  anim.def.mTracks={3,tracks};anim.mTrackInstances=instances;app.reanims[80]=&anim;p->mBodyReanimID=80;
  auto* untouched=SandboxArt::NativeImage("native-test.png");for(auto& track:instances)track.mImageOverride=untouched;
  drawnOverrides.clear();Sexy::Graphics g(nullptr);assert(SandboxPlants::DrawBody(&g,p,0,0));
  const int level=power==180?2:12;assert(drawnOverrides[0]==SandboxArt::PowerNative(entry.image,level,power));
  assert(drawnOverrides[1]==(entry.mouthImage?SandboxArt::PowerNative(entry.mouthImage,level,power):untouched));
  assert(drawnOverrides[2]==untouched);for(const auto& track:instances)assert(track.mImageOverride==untouched);
  app.reanims.clear();
 }
 for(int power:{180,181,182})for(int base:{3,23})for(int stage=0;stage<3;++stage){
  SandboxPlants::Reset();Board w;auto* p=w.plant(1,1);p->mSeedType=static_cast<SeedType>(base);
  p->mPlantMaxHealth=base==3?4000:8000;p->mPlantHealth=p->mPlantMaxHealth*(3-stage)/3;
  SandboxPlants::Assign(p,SandboxMemeRules::Result(base,power));
  Reanimation anim;Track tracks[]={{base==3?"anim_face":"anim_idle"}};TrackInstance instances[1];
  anim.def.mTracks={1,tracks};anim.mTrackInstances=instances;app.reanims[80]=&anim;p->mBodyReanimID=80;
  const std::string path=std::string(base==3?"Wallnut_":"Tallnut_")+(stage==0?"body.png":stage==1?"cracked1.png":"cracked2.png");
  auto* original=SandboxArt::NativeImage(path.c_str());instances[0].mImageOverride=original;
  drawnOverrides.clear();Sexy::Graphics g(nullptr);assert(SandboxPlants::DrawBody(&g,p,0,0));
  assert(drawnOverrides[0]==SandboxArt::PowerNative(path.c_str(),power==180?2:12,power));
  assert(instances[0].mImageOverride==original);app.reanims.clear();
 }
 SandboxPlants::Reset();
 Sexy::Graphics g(nullptr);Reanimation mouth;mouth.track="idle_mouth";mouth.matrix={0,-0.72f,60,0.72f,0,40};
 Board board;auto* pea=board.plant(2,2);SandboxPlants::Assign(pea,120);pea->mHeadReanimID=1;app.reanims[1]=&mouth;
 auto* shot=board.AddProjectile(0,0,0,2,PROJECTILE_PEA);SandboxPlants::OnFired(pea,shot,nullptr);
 near(shot->mPosX+12,pea->mX+60);near(shot->mPosY+12,pea->mY+50.44f);near(SandboxPlants::ShotScale(shot),1);
 assert(SandboxPlants::HasShot(shot)&&SandboxPlants::ShotRadius(shot)==12&&!SandboxPlants::DrawShot(&g,shot)&&!SandboxPlants::Impact(shot,nullptr));
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
  assert(!SandboxPlants::DrawBody(&g,rage,0,0)&&instances[0].mImageOverride==native);
  for(int heat:{20,60,100,140,180}){assert(SandboxPlants::RestorePower(rage,{500,300,0,heat,0,0,0,0,0,2}));drawnOverrides.clear();
   assert(SandboxPlants::DrawBody(&g,rage,0,0));assert(drawnOverrides[0]==SandboxArt::WarmNative("PeaShooter_Head.png",heat*24/200));assert(instances[0].mImageOverride==native);
  }app.reanims.erase(80);
 }
 for(int id=120;id<144;++id){auto* p=board.plant(1,1);p->mSeedType=static_cast<SeedType>(SandboxPlants::Base(id));SandboxPlants::Assign(p,id);
  assert(SandboxPlants::KeepsNativeBlink(p));float x=0,y=0,sx=1,sy=1;SandboxPlants::AdjustScale(p,x,y,sx,sy);near(x+40*sx,40);near(y+65*sy,65);
 }
 for(int base=0;base<48;++base)if(!SandboxMemeRules::LegacyBase(base))for(int power:{180,181,182}){
  auto* p=board.plant(1,1);p->mSeedType=static_cast<SeedType>(base);SandboxPlants::Assign(p,SandboxMemeRules::Result(base,power));
  assert(!SandboxPlants::DrawBody(&g,p,0,0));float x=17,y=23,sx=0.8f,sy=1.2f;SandboxPlants::AdjustScale(p,x,y,sx,sy);
  near(x,17);near(y,23);near(sx,0.8f);near(sy,1.2f);Sexy::Color tint(255,255,255,177);SandboxPlants::NativeTint(p,tint);
  assert(tint.mAlpha==177&&(tint.mRed<255||tint.mGreen<255||tint.mBlue<255));
 }
 std::cout<<"Visual coordinate contracts passed\n";
}
