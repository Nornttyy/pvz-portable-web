#include "AbstractRigVisuals.h"
#include "LawnApp.h"
#include "MemeCharacters.h"
#include "MemeAdventure.h"
#include "Lawn/Plant.h"
#include "Lawn/Board.h"
#include "Lawn/Zombie.h"
#include "SandboxArt.h"
#include "PvzpLib/Reanimator.h"
#include "graphics/GLImage.h"
#include "graphics/Graphics.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <string_view>
extern bool gSandboxEnabled;
namespace AbstractRigVisuals {
namespace {
struct Pose {Reanimation* anim;int kind,pulse;const Plant* plant=nullptr;std::array<int,10> state{};int part=0;bool held=false;};
std::vector<Pose> poses;
Sexy::Image* Art(const char* name){static std::map<std::string,std::unique_ptr<Sexy::GLImage>> cache;auto& im=cache[name];if(!im)im.reset(gLawnApp->GetImage(std::string("/addons/art/")+name+".png"));return im.get();}
Sexy::Image* SquashWithHeadband(){
 static std::unique_ptr<Sexy::MemoryImage> composed;if(composed)return composed.get();
 auto* original=dynamic_cast<Sexy::MemoryImage*>(SandboxArt::NativeImage("Squash_body.png"));auto* band=Art("squash-headband");if(!original||!band)return original;
 composed=std::make_unique<Sexy::MemoryImage>();composed->Create(original->mWidth,original->mHeight);
 std::copy(original->GetBits(),original->GetBits()+original->mWidth*original->mHeight,composed->GetBits());composed->BitsChanged();
 Sexy::Graphics g(composed.get());g.DrawImage(band,0,0);return composed.get();
}
void Rotate(ReanimatorTransform& t,float x,float y,float radians){const float c=std::cos(radians),s=std::sin(radians),dx=t.mTransX-x,dy=t.mTransY-y;t.mTransX=x+c*dx-s*dy;t.mTransY=y+s*dx+c*dy;t.mSkewX+=radians*180/3.14159265f;t.mSkewY+=radians*180/3.14159265f;}
void Warp(ReanimatorTransform& t,float x,float y,float sx,float sy,float angle=0){
 const float kx=t.mSkewX*3.14159265f/180,ky=t.mSkewY*3.14159265f/180;
 const float a=sx*std::cos(kx)*t.mScaleX,c=sy*std::sin(kx)*t.mScaleX,b=-sx*std::sin(ky)*t.mScaleY,d=sy*std::cos(ky)*t.mScaleY;
 t.mTransX=x+sx*(t.mTransX-x);t.mTransY=y+sy*(t.mTransY-y);t.mScaleX=std::hypot(a,c);t.mScaleY=std::hypot(b,d);t.mSkewX=std::atan2(c,a)*180/3.14159265f;t.mSkewY=std::atan2(-b,d)*180/3.14159265f;Rotate(t,x,y,angle);
}
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
 if(track&&a->TrackExists(track)){auto* part=a->GetTrackInstanceByName(track);images.emplace_back(part,part->mImageOverride);part->mImageOverride=kind==3?SandboxArt::NativeImage("Zombie_polevaulter_pole2.png"):kind==518?SquashWithHeadband():Art("bucket-glove");}
 if(kind==3&&pulse>=0&&a->TrackExists("Zombie_polevaulter_pole2")){auto* part=a->GetTrackInstanceByName("Zombie_polevaulter_pole2");groups.emplace_back(part,part->mRenderGroup);part->mRenderGroup=RENDER_GROUP_NORMAL;}
}
Scope::Scope(const Plant* p):mark(poses.size()){
 const int kind=MemeCharacters::Type(p);if(!MemeCharacters::Is(kind)||p->mSquished)return;
 const auto state=MemeCharacters::Save(p);int part=0;
 bool held=false;if(kind==509)for(auto* z:p->mBoard->mZombies)if(!z->mDead&&int(z->mZombiePhase)==1024&&static_cast<unsigned>(z->mTargetPlantID)==p->mBoard->mPlants.DataArrayGetID(const_cast<Plant*>(p)))held=true;
 for(auto id:{p->mBodyReanimID,p->mHeadReanimID,p->mHeadReanimID2,p->mHeadReanimID3,p->mBlinkReanimID}){
  if(auto* a=gLawnApp->ReanimationTryToGet(id)){Add(a,kind,state[7]);auto& pose=poses.back();pose.plant=p;pose.state=state;pose.part=part;pose.held=held;}++part;
 }
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
 if(p->plant&&!p->plant->mIsAsleep){
  const auto& s=p->state;const float pulse=std::sin(std::clamp(s[7]/40.0f,0.0f,1.0f)*3.14159265f);
  const bool head=p->part==1||p->part==4;
  switch(p->kind){
   case 500:if(head){
    const bool burst=s[2]==1;const float heat=burst?1:s[3]/300.0f;
    const float exhaustion=!burst&&s[3]==0&&s[5]>150?(s[5]-150)/150.0f:0;
    const float progress=burst?1-s[8]/50.0f:0;
    Warp(t,38,53,1+.22f*heat,1+.13f*heat-.20f*exhaustion,burst?-.15f-.30f*progress+.04f*std::sin(s[6]*.16f):.18f*exhaustion);
   }return;
   case 501:return;
   case 502:if(name!="PuffShroom_stem")Warp(t,35,53,1+.20f*pulse,1-.10f*pulse,.18f*pulse);return;
   case 503:Rotate(t,40,65,s[2]==1?.20f:s[2]==2?-.25f*pulse:0);return;
   case 504:if(head&&s[2]==1)Rotate(t,38,50,-.45f*std::sin((80-s[4])*3.14159265f/80));return;
   case 505:if(head)Rotate(t,38,52,.25f*pulse);return;
   case 506:if(head&&s[8]&&s[5]>15){const float listen=std::min(1.0f,s[7]/20.0f);if(s[9]<0)Reflect(t,38,1-1.7f*listen);else Rotate(t,38,52,-.2f*listen);}return;
   case 507:if(name=="anim_light"||name=="anim_glow"||name=="PotatoMine_stem")Warp(t,40,55,1,1+.45f*pulse);return;
   case 508:if(name.starts_with("Cactus_arm")){const bool left=name.starts_with("Cactus_arm1");Rotate(t,left?26:47,55,(left?-.9f:.9f)*pulse);}return;
   case 509:{
    const bool held=p->held;
    if(name=="Chomper_stomach")Warp(t,37,64,held?1.65f:1,held?1.3f:1);
    else if(held&&!name.starts_with("Chomper_groundleaf")&&!name.starts_with("Chomper_stem"))Warp(t,40,52,1.15f,1+.06f*std::sin(s[6]*.20f),-.16f);
    return;
   }
   case 510:if(!name.starts_with("Cabbagepult_backleaf")&&!name.starts_with("frontleaf"))Rotate(t,38,67,-.30f*pulse);return;
   case 511:if(name!="Starfruit_stem"&&name!="Starfruit_leaf")Rotate(t,36,35,s[3]*3.14159265f/8);return;
   case 512:Rotate(t,40,66,.28f*pulse);return;
   case 513:if(s[2]==1&&s[7])Warp(t,37,65,1+.3f*pulse,1-.15f*pulse,-.2f*pulse);return;
   case 514:if(name=="DoomShroom_spout"||name=="DoomShroom_tip")Warp(t,40,45,1+.55f*s[3]/300.0f,1+.30f*s[3]/300.0f);return;
   case 515:if(p->part==1||p->part==2){const float sign=p->part==1?-1:1;Rotate(t,38,53,sign*.40f*pulse);}return;
   case 516:if(s[7])Warp(t,40,70,1-.18f*pulse,1+.45f*pulse);return;
  }
 }
 // Original squash face/stem/blinks remain intact; only the headband is generated.
 if(p->kind==518)return;
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
