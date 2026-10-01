#include "AbstractRigVisuals.h"
#include "LawnApp.h"
#include "MemeCharacters.h"
#include "MemeShooterRules.h"
#include "SandboxArt.h"
#include "SandboxZombies.h"
#include "ConeBodyRules.h"
#include "Resources.h"
#include "graphics/Image.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "PvzpLib/Reanimator.h"
#include <algorithm>
#include <cmath>
#include <string_view>
namespace AbstractRigVisuals {
namespace {
struct Pose {Reanimation* anim;const Plant* plant;std::array<int,10> state;int part;};
std::vector<Pose> poses;
void ConePose(Scope& scope,Reanimation* a,int armor){
 if(!a)return; // Even with zero armor, the body remains made of cones.
 std::array<int,10> state{};state[0]=SandboxZombies::ConeWrap;state[1]=armor;poses.push_back({a,nullptr,state,0});
 for(int i=0;i<a->mDefinition->mTracks.count;++i){
  const int part=ConeBodyRules::Index(a->mDefinition->mTracks.tracks[i].mName);
  auto* track=&a->mTrackInstances[i];scope.groups.push_back({track,track->mRenderGroup});
  track->mRenderGroup=part>=0?RENDER_GROUP_NORMAL:RENDER_GROUP_HIDDEN;
  if(part>=0){scope.images.push_back({track,track->mImageOverride});track->mImageOverride=nullptr;}
 }
}
void Rotate(ReanimatorTransform& t,float x,float y,float radians){const float c=std::cos(radians),s=std::sin(radians),dx=t.mTransX-x,dy=t.mTransY-y;t.mTransX=x+c*dx-s*dy;t.mTransY=y+s*dx+c*dy;t.mSkewX+=radians*180/3.14159265f;t.mSkewY+=radians*180/3.14159265f;}
void Warp(ReanimatorTransform& t,float x,float y,float sx,float sy,float angle=0){
 const float kx=t.mSkewX*3.14159265f/180,ky=t.mSkewY*3.14159265f/180;
 const float a=sx*std::cos(kx)*t.mScaleX,c=sy*std::sin(kx)*t.mScaleX,b=-sx*std::sin(ky)*t.mScaleY,d=sy*std::cos(ky)*t.mScaleY;
 t.mTransX=x+sx*(t.mTransX-x);t.mTransY=y+sy*(t.mTransY-y);t.mScaleX=std::hypot(a,c);t.mScaleY=std::hypot(b,d);t.mSkewX=std::atan2(c,a)*180/3.14159265f;t.mSkewY=std::atan2(-b,d)*180/3.14159265f;Rotate(t,x,y,angle);
}
}
void Scope::Add(Reanimation*,int,int){}
Scope::Scope(const Plant* p):mark(poses.size()){
 if((MemeCharacters::Type(p)!=500&&MemeCharacters::Type(p)!=MemeCharacters::ShooterPea&&MemeCharacters::Type(p)!=MemeCharacters::TuckingSunflower)||p->mSquished)return;
 const auto state=MemeCharacters::Save(p);int part=0;
 for(auto id:{p->mBodyReanimID,p->mHeadReanimID,p->mHeadReanimID2,p->mHeadReanimID3,p->mBlinkReanimID}){
  if(auto* a=gLawnApp->ReanimationTryToGet(id))poses.push_back({a,p,state,part});
  ++part;
 }
}
// Identity gates keep ordinary coneheads and unrelated cached previews intact.
Scope::Scope(Zombie* z):mark(poses.size()){
 if(SandboxZombies::IsConeWrap(z))ConePose(*this,gLawnApp->ReanimationTryToGet(z->mBodyReanimID),z->mHelmHealth);
}
Scope::Scope(Reanimation* a,int type):mark(poses.size()){
 if(type==SandboxZombies::ConeWrap)ConePose(*this,a,SandboxZombies::ConeCount*SandboxZombies::ConeHealth);
 if(a&&type==MemeCharacters::ShooterPea){std::array<int,10> state{};state[0]=type;poses.push_back({a,nullptr,state,1});}
 // Cards always show the standing pose; only live plants can tuck their head.
}
Scope::~Scope(){for(auto& [track,image]:images)track->mImageOverride=image;for(auto& [track,group]:groups)track->mRenderGroup=group;poses.resize(mark);}
void Transform(Reanimation* a,int track,ReanimatorTransform& t){
 const Pose* p=nullptr;for(auto i=poses.rbegin();i!=poses.rend();++i)if(i->anim==a){p=&*i;break;}
 if(!p)return;
 if(p->state[0]==SandboxZombies::ConeWrap){
  const int part=ConeBodyRules::Index(a->mDefinition->mTracks.tracks[track].mName);
  if(part<0){t.mAlpha=0;return;}
  // Native eating hides its tongue. This track now carries a structural cone,
  // so retain it; submerged leg tracks still use the original frame/clip gates.
  if(part==4&&t.mFrame<0)t.mFrame=0;
  if(t.mFrame<0)return;
  const int hp=SandboxZombies::ConeVisualHealth(p->state[1],part);
  const char* files[]={"Zombie_cone1.png","Zombie_cone2.png","Zombie_cone3.png"};
  auto* image=SandboxArt::NativeImage(files[SandboxZombies::ConeDamageStage(hp)]);if(!image)return;
  const auto& fit=ConeBodyRules::Parts[part];
  const float kx=t.mSkewX*3.14159265f/180,ky=t.mSkewY*3.14159265f/180;
  const float dx=fit.cx-fit.width*.5f,dy=fit.cy-fit.height*.5f;
  t.mTransX+=dx*t.mScaleX*std::cos(kx)-dy*t.mScaleY*std::sin(ky);
  t.mTransY+=dx*t.mScaleX*std::sin(kx)+dy*t.mScaleY*std::cos(ky);
  t.mScaleX*=fit.width/image->mWidth;t.mScaleY*=fit.height/image->mHeight;t.mImage=image;
  return;
 }
 if(p->state[0]==MemeCharacters::TuckingSunflower){
  if(!p->state[2])return;
  const std::string_view name=a->mDefinition->mTracks.tracks[track].mName;
  // Retract the stalk and lower the entire head, keeping its round face and
  // petals together. Roots/leaves stay planted; never squash the whole sprite.
  if(name=="anim_idle"||name.starts_with("anim_blink")||name.starts_with("SunFlower_")){
   Warp(t,37,45,.80f,.80f);t.mTransY+=24;
  }else if(name.starts_with("stalk_"))Warp(t,37,80,1,.35f);
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
 const float progress=burst?1-float(s[8])/MemeShooterRules::BurstCount:0;
 Warp(t,38,53,1+.22f*heat,1+.13f*heat-.20f*exhaustion,burst?-.15f-.30f*progress+.04f*std::sin(s[6]*.16f):.18f*exhaustion);
}
}
