// Native walking/eating/damage bones with per-instance original costumes and abilities.
#include "SandboxZombies.h"
#include "SandboxCombatRules.h"
#include "SandboxArt.h"
#include "SandboxRules.h"
#include "Sandbox.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "Lawn/System/ReanimationLawn.h"
#include "PvzpLib/Reanimator.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <string_view>
namespace SandboxZombies {
namespace {
struct State {int id,age=0,burst=0;bool hadArmor=true,charged=false,split=false;int repairFlash=0;};
struct Birth {int row;float x;};
std::map<const Zombie*,State> states;
std::map<const Zombie*,float> auras;
std::vector<Birth> births;
std::map<int,std::unique_ptr<Sexy::MemoryImage>> portraits;
void Skin(Reanimation* a,int id){
 if(!a)return;auto* d=Find(id);if(!d)return;
 for(int i=0;i<a->mDefinition->mTracks.count;++i){
  const std::string_view n=a->mDefinition->mTracks.tracks[i].mName;auto& t=a->mTrackInstances[i];
  if(n=="anim_head1")t.mImageOverride=SandboxArt::Image(d->art,"head");
  else if(n=="Zombie_body")t.mImageOverride=SandboxArt::Image(d->art,"body");
  else if(n=="anim_head2")t.mImageOverride=SandboxArt::Image(d->art,"jaw");
  else if(n=="anim_cone"&&d->base==2)t.mImageOverride=SandboxArt::Image(d->art,"prop");
  else if(n=="anim_bucket"&&d->base==4)t.mImageOverride=SandboxArt::Image(d->art,"prop");
  else if(n=="Zombie_tie"&&id==204)t.mImageOverride=SandboxArt::Image(d->art,"battery");
  else if(n=="anim_hair"&&(id==205||id==208||id==209))t.mImageOverride=SandboxArt::Image(d->art,"hat");
 }
}
void SkinFlag(Reanimation* a,int id){
 if(!a||id!=201)return;
 if(a->TrackExists("Zombie_flag"))a->SetImageOverride("Zombie_flag",SandboxArt::Image(Find(id)->art,"prop"));
}
bool Alive(Zombie* z){return !z->mMindControlled&&!z->IsDeadOrDying()&&z->mHasHead;}
void Point(Zombie* z,const char* bone,float w,float h,float& x,float& y){
 float px,py;if(SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(z->mBodyReanimID),bone,w,h,w*0.5f,h*0.5f,px,py)){
  x=z->mX+px;y=z->mY+py;
 }
}
}
void Reset(){states.clear();auras.clear();births.clear();}
void Forget(Zombie* z){states.erase(z);auras.erase(z);}
void Assign(Zombie* z,int id){
 auto* d=Find(id);if(!d)return;states[z]={id};
 z->mBodyHealth=z->mBodyMaxHealth=d->health;
 if(d->armor)z->mHelmHealth=z->mHelmMaxHealth=d->armor;
 z->mScaleZombie=SandboxCombatRules::Scale(id);
 auto* a=gLawnApp->ReanimationTryToGet(z->mBodyReanimID);Skin(a,id);
 if(id==201)SkinFlag(gLawnApp->ReanimationTryToGet(z->mSpecialHeadReanimID),id);
 z->UpdateReanim();
}
float Speed(const Zombie* z){
 if(!gSandboxEnabled)return 1;
 auto it=states.find(z);float speed=1;
 if(it!=states.end()){const auto& s=it->second;speed=SandboxCombatRules::Speed(s.id,z->mHelmHealth>0,s.burst,s.charged);}
 auto aura=auras.find(z);return speed*(aura==auras.end()?1:aura->second);
}
int Damage(const Zombie* z,int damage){
 auto it=states.find(z);return gSandboxEnabled&&it!=states.end()&&it->second.id==208?SandboxCombatRules::SmokeDamage(damage,it->second.age):damage;
}
bool ElectricHit(Zombie* z){
 auto it=states.find(z);if(it==states.end()||it->second.id!=204||it->second.charged)return false;
 it->second.charged=true;z->mBodyHealth=std::min(z->mBodyMaxHealth,z->mBodyHealth+80);return true;
}
void CombatDeath(Zombie* z){
 auto it=states.find(z);if(!gSandboxEnabled||it==states.end()||it->second.id!=209||it->second.split||z->mMindControlled)return;
 it->second.split=true;births.push_back({z->mRow,z->mPosX});
}
void Tick(Board* b){
 if(b->mPaused)return;
 auras.clear();
 // Spawn after the damage iteration, with the same arena cap as manual placement.
 for(const auto birth:births)for(int i=0;i<2;++i){
  int live=0;for(auto* z:b->mZombies)if(!z->IsDeadOrDying())++live;
  if(live>=SandboxRules::MaxZombies||b->mZombies.mSize>=b->mZombies.mMaxSize-8)break;
  Zombie::PreloadZombieResources(ZOMBIE_IMP);
  if(auto* imp=b->AddZombieInRow(ZOMBIE_IMP,birth.row,Zombie::ZOMBIE_WAVE_DEBUG)){
   imp->mPosX=std::clamp(birth.x+i*24,20.0f,850.0f);imp->mX=int(imp->mPosX);imp->UpdateReanim();
  }
 }
 births.clear();
 for(auto* z:b->mZombies){
  auto it=states.find(z);if(it==states.end())continue;if(!Alive(z))continue;auto& s=it->second;++s.age;
  if(s.repairFlash>0)--s.repairFlash;
  if(s.burst>0)--s.burst;
  if(s.id==200&&s.hadArmor&&z->mHelmHealth<=0){s.hadArmor=false;s.burst=300;}
  if(s.id==201&&s.age%600<220){
   for(auto* other:b->mZombies)if(other!=z&&Alive(other)&&std::abs(other->mRow-z->mRow)<=1&&std::abs(other->mPosX-z->mPosX)<150)auras[other]=1.35f;
  }
  if(s.id==203&&z->mHelmHealth>0)z->RemoveColdEffects();
  if(s.id==207&&s.age%400==0){
   Zombie* repair=nullptr;int missing=0;
   for(auto* other:b->mZombies)if(other!=z&&Alive(other)&&std::abs(other->mRow-z->mRow)<=1&&std::abs(other->mPosX-z->mPosX)<160&&other->mHelmHealth>0){
    const int need=other->mHelmMaxHealth-other->mHelmHealth;if(need>missing){missing=need;repair=other;}
   }
   if(repair){repair->mHelmHealth=SandboxCombatRules::Repair(repair->mHelmHealth,repair->mHelmMaxHealth);s.repairFlash=35;}
  }
 }
 // One slow per plant; several gum zombies cannot freeze a launch counter forever.
 if(b->mMainCounter%3==0)for(auto* p:b->mPlants)if(!p->mDead&&!p->mIsAsleep&&SandboxCombatRules::CanSlow(p->mLaunchCounter)&&GetPlantDefinition(p->mSeedType).mSubClass==SUBCLASS_SHOOTER){
  for(const auto& [z,s]:states)if(s.id==202&&!z->mDead&&z->mHasHead&&z->mBodyHealth>0&&!z->mMindControlled&&z->mRow==p->mRow&&std::abs(z->mPosX+40-(p->mX+40))<150){++p->mLaunchCounter;break;}
 }
}
void DrawPortrait(Sexy::Graphics* g,int x,int y,int w,int h,int id){
 auto* d=Find(id);if(!d)return;auto& image=portraits[id];
 if(!image){
  image=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(220,240);Sexy::Graphics canvas(image.get());canvas.SetLinearBlend(true);
  Reanimation a;a.ReanimationInitializeType(50,50,REANIM_ZOMBIE);a.SetFramesForLayer("anim_idle");Zombie::SetupReanimLayers(&a,static_cast<ZombieType>(d->base));Skin(&a,id);
  if(d->base==1&&a.TrackExists("Zombie_flaghand")){
   Reanimation flag;flag.ReanimationInitializeType(0,0,REANIM_FLAG);flag.SetFramesForLayer("Zombie_flag");SkinFlag(&flag,id);
   a.mFrameBasePose=0;a.GetAttachmentOverlayMatrix(a.FindTrackIndex("Zombie_flaghand"),flag.mOverlayMatrix);flag.Draw(&canvas);
  }
  a.Draw(&canvas);
  if(id==202){float px,py;if(SandboxArt::TrackPoint(&a,"anim_head2",32,15,4,7,px,py))SandboxArt::Sprite(&canvas,"gum",px-4,py,18,18);}
 }
 SandboxArt::DrawFit(g,image.get(),x+5,y+3,w-10,h-6,id==205?0.86f:1.0f);
}
void DrawEffects(Sexy::Graphics* graphics,Board* b,int row){
 Sexy::Graphics clipped(*graphics);clipped.ClipRect(0,82,800,518);auto* g=&clipped;
 for(const auto& [z,s]:states)if(!z->mDead&&z->mHasHead&&z->mRow==row&&z->mBodyHealth>0){
  float hx=z->mX+45,hy=z->mY+30;Point(const_cast<Zombie*>(z),"anim_head1",53,48,hx,hy);
  float bx=z->mX+45,by=z->mY+65;Point(const_cast<Zombie*>(z),"Zombie_body",53,63,bx,by);
  if(s.id==208&&s.age%700<240){
   const int phase=s.age%60,frame=phase/20;const char* art=frame==0?"smoke-0":frame==1?"smoke-1":"smoke-2";
   SandboxArt::Sprite(g,art,bx,by-phase*0.3f,65*z->mScaleZombie,65*z->mScaleZombie,0,120);
  }
  if(s.id==201&&s.age%600<220)SandboxArt::Sprite(g,"notes",hx-20,hy-34-(s.age%40)*0.2f,18,23,0,200);
  if(s.id==202){float x=hx-12,y=hy+20;Point(const_cast<Zombie*>(z),"anim_head2",32,15,x,y);const float size=(17+2*std::sin(s.age*0.08f))*z->mScaleZombie;SandboxArt::Sprite(g,"gum",x-14*z->mScaleZombie,y,size,size,0,230);}
  if(s.id==204&&s.charged)SandboxArt::Sprite(g,"charge",bx,by,26,26,0,155);
  if(s.repairFlash>0)SandboxArt::Sprite(g,"repair",hx+20,hy-8,22,22,0,std::min(230,s.repairFlash*18));
 }
}
}
