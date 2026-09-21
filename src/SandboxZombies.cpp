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
struct State {int id,age=0,burst=0;bool hadArmor=true,charged=false,split=false;};
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
  else if(n=="anim_head2")t.mImageOverride=SandboxArt::Image(d->art,id==202?"gum":"jaw");
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
}
void Reset(){states.clear();auras.clear();births.clear();}
void Forget(Zombie* z){states.erase(z);auras.erase(z);}
void Assign(Zombie* z,int id){
 auto* d=Find(id);if(!d)return;states[z]={id};
 z->mBodyHealth=z->mBodyMaxHealth=d->health;
 if(d->armor)z->mHelmHealth=z->mHelmMaxHealth=d->armor;
 z->mScaleZombie=SandboxCombatRules::Scale(id);
 auto* a=gLawnApp->ReanimationTryToGet(z->mBodyReanimID);Skin(a,id);
 if(a)SkinFlag(a->FindSubReanim(REANIM_FLAG),id);
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
   if(repair)repair->mHelmHealth=SandboxCombatRules::Repair(repair->mHelmHealth,repair->mHelmMaxHealth);
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
  if(d->base==1){Reanimation flag;flag.ReanimationInitializeType(50,50,REANIM_FLAG);flag.SetFramesForLayer("Zombie_flag");SkinFlag(&flag,id);flag.Draw(&canvas);}
  a.Draw(&canvas);
 }
 SandboxArt::DrawFit(g,image.get(),x+5,y+3,w-10,h-6,id==205?0.86f:1.0f);
}
void DrawEffects(Sexy::Graphics* g,Board* b){
 for(const auto& [z,s]:states)if(!z->mDead&&z->mHasHead){
  if(s.id==208&&s.age%700<240){
   g->SetColor(Sexy::Color(95,93,91,90));
   for(int i=0;i<3;++i){int offset=(s.age/4+i*12)%36;g->DrawLine(z->mPosX+25,z->mPosY+100-offset,z->mPosX+70,z->mPosY+95-offset);}
  }
  if(s.id==201&&s.age%600<220){g->SetColor(Sexy::Color(232,206,92,170));g->DrawLine(z->mPosX+22,z->mPosY+10,z->mPosX+16,z->mPosY+4);g->DrawLine(z->mPosX+15,z->mPosY+20,z->mPosX+7,z->mPosY+18);}
 }
}
}
