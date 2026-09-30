#include "AbstractRigVisuals.h"
#include "LawnApp.h"
#include "MemeCharacters.h"
#include "MemeAdventure.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "SandboxArt.h"
#include "PvzpLib/Reanimator.h"
#include "graphics/GLImage.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <string_view>
extern bool gSandboxEnabled;
namespace AbstractRigVisuals {
namespace {
struct Pose {Reanimation* anim;int kind,pulse;};
std::vector<Pose> poses;
Sexy::Image* Art(const char* name){static std::map<std::string,std::unique_ptr<Sexy::GLImage>> cache;auto& im=cache[name];if(!im)im.reset(gLawnApp->GetImage(std::string("/addons/art/")+name+".png"));return im.get();}
void Rotate(ReanimatorTransform& t,float x,float y,float radians){const float c=std::cos(radians),s=std::sin(radians),dx=t.mTransX-x,dy=t.mTransY-y;t.mTransX=x+c*dx-s*dy;t.mTransY=y+s*dx+c*dy;t.mSkewX+=radians*180/3.14159265f;t.mSkewY+=radians*180/3.14159265f;}
void Reflect(ReanimatorTransform& t,float pivot,float f){
 const float kx=t.mSkewX*3.14159265f/180,ky=t.mSkewY*3.14159265f/180;
 const float a=f*std::cos(kx)*t.mScaleX,c=std::sin(kx)*t.mScaleX,b=-f*std::sin(ky)*t.mScaleY,d=std::cos(ky)*t.mScaleY;
 t.mTransX=pivot+f*(t.mTransX-pivot);t.mScaleX=std::hypot(a,c);t.mScaleY=std::hypot(b,d);t.mSkewX=std::atan2(c,a)*180/3.14159265f;t.mSkewY=std::atan2(-b,d)*180/3.14159265f;
}
ReanimatorTransform Raw(Reanimation* a,const char* name){ReanimatorTransform t;ReanimatorFrameTime time;a->GetFrameTime(&time);a->GetTransformAtTime(a->FindTrackIndex(name),&t,&time);return t;}
}
void Scope::Add(Reanimation* a,int kind,int pulse){
 if(!a)return;poses.push_back({a,kind,pulse});
 const char* track=kind==518?"Squash_body":kind==4?"Zombie_outerarm_hand":kind==3&&pulse>=0?"Zombie_polevaulter_pole2":nullptr;
 if(track&&a->TrackExists(track)){auto* part=a->GetTrackInstanceByName(track);images.emplace_back(part,part->mImageOverride);part->mImageOverride=kind==3?SandboxArt::NativeImage("Zombie_polevaulter_pole2.png"):Art(kind==518?"squash-exercise":"bucket-glove");}
 if(kind==3&&pulse>=0&&a->TrackExists("Zombie_polevaulter_pole2")){auto* part=a->GetTrackInstanceByName("Zombie_polevaulter_pole2");groups.emplace_back(part,part->mRenderGroup);part->mRenderGroup=RENDER_GROUP_NORMAL;}
}
Scope::Scope(const Plant* p):mark(poses.size()){
 const int kind=MemeCharacters::Type(p);if(kind!=517&&kind!=518)return;
 const int pulse=MemeCharacters::Save(p)[7];
 Add(gLawnApp->ReanimationTryToGet(p->mBodyReanimID),kind,pulse);
 if(kind==517)for(auto id:{p->mHeadReanimID,p->mHeadReanimID2,p->mHeadReanimID3})Add(gLawnApp->ReanimationTryToGet(id),kind,pulse);
}
Scope::Scope(Zombie* z):mark(poses.size()){
 if(!(gSandboxEnabled||gLawnApp->IsAdventureMode())||!z->mHasArm||z->IsDeadOrDying())return;
 const int kind=int(z->mZombieType);if(kind!=4&&kind!=3)return;
 const int pulse=kind==4?(z->mZombiePhase==PHASE_ZOMBIE_NORMAL?z->mPhaseCounter:0):(z->mZombiePhase==PHASE_POLEVAULTER_POST_VAULT?z->mPhaseCounter:-1);
 Add(gLawnApp->ReanimationTryToGet(z->mBodyReanimID),kind,pulse);
}
Scope::Scope(Reanimation* a,int previewBase):mark(poses.size()){
 if(!gSandboxEnabled&&!MemeAdventure::RosterEnabled())return;
 if(previewBase==17)Add(a,518);else if(previewBase==18)Add(a,517);else if(previewBase==1004)Add(a,4);
}
Scope::~Scope(){for(auto it=images.rbegin();it!=images.rend();++it)it->first->mImageOverride=it->second;for(auto it=groups.rbegin();it!=groups.rend();++it)it->first->mRenderGroup=it->second;poses.resize(mark);}
void Transform(Reanimation* a,int index,ReanimatorTransform& t){
 const Pose* p=nullptr;for(auto i=poses.rbegin();i!=poses.rend();++i)if(i->anim==a){p=&*i;break;}if(!p)return;
 const std::string_view name=a->mDefinition->mTracks.tracks[index].mName;
 if(p->kind==518){if(name=="Squash_stem"||name=="anim_face"||name=="anim_eye"||name=="anim_blink")t.mAlpha=0;return;}
 if(p->kind==517){
  int head=0;for(int n=1;n<=3;n++){const std::string digit=std::to_string(n);if(name=="anim_face"+digit||name=="ThreePeater_mouth"+digit||name.starts_with("ThreePeater_head"+digit+"_")){head=n;break;}}
  if(!head)return;const std::string face="anim_face"+std::to_string(head);if(!a->TrackExists(face.c_str()))return;
  const auto anchor=Raw(a,face.c_str());
  // The complete head, mouth, blink and leaves turn together; never detach a muzzle.
  const float turn=p->pulse>0?std::sin((50-p->pulse)*3.14159265f/50):0;
  if(turn>0)Reflect(t,anchor.mTransX+9,1-1.92f*turn);
  if(name.find("leaf")!=std::string_view::npos){t.mScaleX*=1.65f;t.mScaleY*=1.25f;}
  return;
 }
 if(p->kind==4&&p->pulse>0&&p->pulse<=40&&name.starts_with("Zombie_outerarm_")){
  const auto shoulder=Raw(a,"Zombie_outerarm_upper");const float t0=(40-p->pulse)/40.0f;
  Rotate(t,shoulder.mTransX+7,shoulder.mTransY+3,1.5f*std::sin(t0*3.14159265f));
 }
 if(p->kind==3&&p->pulse>=0){
  const float amount=p->pulse<=70?std::sin((70-p->pulse)*3.14159265f/140):0;
  if(name=="Zombie_polevaulter_pole2"){
   // Reuse the original pole as a folded spare on his back. Pull it upwards
   // with the arm before the native running/holding animation takes over.
   t.mFrame=0;t.mAlpha=1;t.mTransX=70;t.mTransY=104-48*amount;
   t.mScaleX=.70f;t.mScaleY=1.0f;t.mSkewX=t.mSkewY=-78+65*amount;
  }
  if(name=="Zombie_polevaulter_outerarm_upper"||name=="Zombie_polevaulter_outerarm_lower"||name=="Zombie_outerarm_hand"){
   const auto shoulder=Raw(a,"Zombie_polevaulter_outerarm_upper");Rotate(t,shoulder.mTransX+7,shoulder.mTransY+2,-1.8f*amount);
  }
 }
}
}
