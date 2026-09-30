#include "AbstractRigVisuals.h"
#include "LawnApp.h"
#include "MemeCharacters.h"
#include "SandboxArt.h"
#include "Resources.h"
#include "graphics/Image.h"
#include "Lawn/Plant.h"
#include "PvzpLib/Reanimator.h"
#include <algorithm>
#include <cmath>
#include <string_view>
namespace AbstractRigVisuals {
namespace {
struct Pose {Reanimation* anim;const Plant* plant;std::array<int,10> state;int part;};
std::vector<Pose> poses;
void Rotate(ReanimatorTransform& t,float x,float y,float radians){const float c=std::cos(radians),s=std::sin(radians),dx=t.mTransX-x,dy=t.mTransY-y;t.mTransX=x+c*dx-s*dy;t.mTransY=y+s*dx+c*dy;t.mSkewX+=radians*180/3.14159265f;t.mSkewY+=radians*180/3.14159265f;}
void Warp(ReanimatorTransform& t,float x,float y,float sx,float sy,float angle=0){
 const float kx=t.mSkewX*3.14159265f/180,ky=t.mSkewY*3.14159265f/180;
 const float a=sx*std::cos(kx)*t.mScaleX,c=sy*std::sin(kx)*t.mScaleX,b=-sx*std::sin(ky)*t.mScaleY,d=sy*std::cos(ky)*t.mScaleY;
 t.mTransX=x+sx*(t.mTransX-x);t.mTransY=y+sy*(t.mTransY-y);t.mScaleX=std::hypot(a,c);t.mScaleY=std::hypot(b,d);t.mSkewX=std::atan2(c,a)*180/3.14159265f;t.mSkewY=std::atan2(-b,d)*180/3.14159265f;Rotate(t,x,y,angle);
}
}
void Scope::Add(Reanimation*,int,int){}
Scope::Scope(const Plant* p):mark(poses.size()){
 if((MemeCharacters::Type(p)!=500&&MemeCharacters::Type(p)!=MemeCharacters::ShooterPea&&MemeCharacters::Type(p)!=MemeCharacters::AwkwardSunflower)||p->mSquished)return;
 const auto state=MemeCharacters::Save(p);int part=0;
 for(auto id:{p->mBodyReanimID,p->mHeadReanimID,p->mHeadReanimID2,p->mHeadReanimID3,p->mBlinkReanimID}){
  if(auto* a=gLawnApp->ReanimationTryToGet(id))poses.push_back({a,p,state,part});
  ++part;
 }
}
// Retired characters must not alter native zombies or cached/almanac previews.
Scope::Scope(Zombie*):mark(poses.size()){}
Scope::Scope(Reanimation* a,int type):mark(poses.size()){
 if(a&&type==MemeCharacters::ShooterPea){std::array<int,10> state{};state[0]=type;poses.push_back({a,nullptr,state,1});}
 if(a&&type==MemeCharacters::AwkwardSunflower){std::array<int,10> state{};state[0]=type;state[4]=MemeCharacters::AwkwardDuration;poses.push_back({a,nullptr,state,0});}
}
Scope::~Scope(){poses.resize(mark);}
void Transform(Reanimation* a,int track,ReanimatorTransform& t){
 const Pose* p=nullptr;for(auto i=poses.rbegin();i!=poses.rend();++i)if(i->anim==a){p=&*i;break;}
 if(!p)return;
 if(p->state[0]==MemeCharacters::AwkwardSunflower){
  const auto& s=p->state;const std::string_view name=a->mDefinition->mTracks.tracks[track].mName;
  const bool face=name=="anim_idle",blink=name.starts_with("anim_blink"),petal=name.starts_with("SunFlower_");
  if(!face&&!blink&&!petal)return; // Never detach stem, roots or leaves.
  if(face&&s[4]>0)if(auto* image=SandboxArt::AwkwardFace())t.mImage=image;
  float dx=0,dy=0;
  if(p->plant&&s[3]>0&&s[5]>0){
   const float blend=std::min({1.0f,(MemeCharacters::AwkwardDuration-s[5])/22.0f,s[5]/35.0f});
   dx=((s[3]-1)%9-p->plant->mPlantCol)*blend;dy=((s[3]-1)/9-p->plant->mRow)*blend;
  }
  Warp(t,37,53,1-.13f*std::abs(dx),1,dx*.24f);
  t.mTransX+=dx*(face||blink?5:2);t.mTransY+=dy*4;
  if(s[4]>0){const float q=std::min({1.0f,(MemeCharacters::AwkwardDuration-s[4])/20.0f,s[4]/30.0f});t.mTransY+=2.5f*q;}
  return;
 }
 if(p->state[0]==MemeCharacters::ShooterPea){
  const std::string_view name=a->mDefinition->mTracks.tracks[track].mName;
  if(name=="idle_mouth"||name.starts_with("idle_headleaf")||name=="idle_shoot_blink"||name.starts_with("anim_blink")||name=="PeaShooter_eyebrow"){t.mAlpha=0;return;}
  if(!name.starts_with("anim_face")||!Sexy::IMAGE_PROJECTILEPEA)return;
  // Preserve the native head bone's centre, sway and recoil. Both the image
  // AND its pivot become the original round pea, not a resized face texture.
  auto* pea=Sexy::IMAGE_PROJECTILEPEA;
  const float kx=t.mSkewX*3.14159265f/180,ky=t.mSkewY*3.14159265f/180;
  const float cx=t.mTransX+35*t.mScaleX*std::cos(kx)-32.5f*t.mScaleY*std::sin(ky);
  const float cy=t.mTransY+35*t.mScaleX*std::sin(kx)+32.5f*t.mScaleY*std::cos(ky);
  t.mScaleX*=84.0f/pea->mWidth;t.mScaleY*=93.0f/pea->mHeight;t.mImage=pea;
  t.mTransX=cx-pea->mWidth*.5f*t.mScaleX*std::cos(kx)+pea->mHeight*.5f*t.mScaleY*std::sin(ky);
  t.mTransY=cy-pea->mWidth*.5f*t.mScaleX*std::sin(kx)-pea->mHeight*.5f*t.mScaleY*std::cos(ky);
  return;
 }
 if(!p->plant||p->plant->mIsAsleep||(p->part!=1&&p->part!=4))return;
 const auto& s=p->state;
 const bool burst=s[2]==1;const float heat=burst?1:s[3]/300.0f;
 const float exhaustion=!burst&&s[3]==0&&s[5]>150?(s[5]-150)/150.0f:0;
 const float progress=burst?1-s[8]/50.0f:0;
 Warp(t,38,53,1+.22f*heat,1+.13f*heat-.20f*exhaustion,burst?-.15f-.30f*progress+.04f*std::sin(s[6]*.16f):.18f*exhaustion);
}
}
