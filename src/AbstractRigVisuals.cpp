#include "AbstractRigVisuals.h"
#include "LawnApp.h"
#include "MemeCharacters.h"
#include "Lawn/Plant.h"
#include "PvzpLib/Reanimator.h"
#include <algorithm>
#include <cmath>
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
 if(MemeCharacters::Type(p)!=500||p->mSquished)return;
 const auto state=MemeCharacters::Save(p);int part=0;
 for(auto id:{p->mBodyReanimID,p->mHeadReanimID,p->mHeadReanimID2,p->mHeadReanimID3,p->mBlinkReanimID}){
  if(auto* a=gLawnApp->ReanimationTryToGet(id))poses.push_back({a,p,state,part});
  ++part;
 }
}
// Retired characters must not alter native zombies or cached/almanac previews.
Scope::Scope(Zombie*):mark(poses.size()){}
Scope::Scope(Reanimation*,int):mark(poses.size()){}
Scope::~Scope(){poses.resize(mark);}
void Transform(Reanimation* a,int,ReanimatorTransform& t){
 const Pose* p=nullptr;for(auto i=poses.rbegin();i!=poses.rend();++i)if(i->anim==a){p=&*i;break;}
 if(!p||p->plant->mIsAsleep||(p->part!=1&&p->part!=4))return;
 const auto& s=p->state;
 const bool burst=s[2]==1;const float heat=burst?1:s[3]/300.0f;
 const float exhaustion=!burst&&s[3]==0&&s[5]>150?(s[5]-150)/150.0f:0;
 const float progress=burst?1-s[8]/50.0f:0;
 Warp(t,38,53,1+.22f*heat,1+.13f*heat-.20f*exhaustion,burst?-.15f-.30f*progress+.04f*std::sin(s[6]*.16f):.18f*exhaustion);
}
}
