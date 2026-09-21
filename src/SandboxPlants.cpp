// Original sandbox roster; skins and combat state are per instance and never saved into adventure.
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "SandboxArt.h"
#include "SandboxCombatRules.h"
#include "Sandbox.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "Lawn/Projectile.h"
#include "Lawn/SeedPacket.h"
#include "Lawn/System/ReanimationLawn.h"
#include "PvzpLib/Reanimator.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <cmath>
#include <algorithm>
namespace SandboxPlants {
namespace {
struct State { int id,shots=0,age=0,cooldown=0,health=0,echo=-1;bool closed=false; };
struct Shot { int id,damage; };
struct Beam { float x1,y1,x2,y2;int ticks;bool electric; };
std::map<const Plant*,State> states;
std::map<const Projectile*,Shot> shots;
std::map<ZombieID,int> poison;
std::map<int,std::unique_ptr<Sexy::MemoryImage>> cards;
std::vector<Beam> beams;
bool Enemy(Zombie* z){return !z->mMindControlled&&!z->IsDeadOrDying()&&z->mHasHead;}
void Skin(Reanimation* anim,int id,bool closed,int health=4000){
 if(!anim)return;
 const auto* def=Find(id);if(!def)return;
 const bool triple=def->base==18;
 const char* family=def->art?def->art:(def->element==Element::Fire?"fire":"ice");
 for(int i=0;i<anim->mDefinition->mTracks.count;++i){
  const std::string_view name=anim->mDefinition->mTracks.tracks[i].mName;
  auto& t=anim->mTrackInstances[i];
  if(id==109){
   if(name=="anim_face")t.mImageOverride=SandboxArt::Image(family,health<850?"cracked2":health<1700?"cracked1":closed?"blink":"head");
   if(name.find("blink")!=name.npos)t.mRenderGroup=RENDER_GROUP_HIDDEN;
  }else if(id==110){
   if(name=="anim_idle")t.mImageOverride=SandboxArt::Image(family,closed?"blink":"head");
   if(name.find("blink")!=name.npos)t.mRenderGroup=RENDER_GROUP_HIDDEN;
  }else if(id==111){
   if(name=="anim_face")t.mImageOverride=SandboxArt::Image(family,closed?"blink":"head");
   if(name=="PuffShroom_head")t.mImageOverride=SandboxArt::Image(family,"cap");
   if(name=="PuffShroom_stem")t.mImageOverride=SandboxArt::Image(family,"stem");
   if(name=="PuffShroom_eyes"||name.find("blink")!=name.npos)t.mRenderGroup=RENDER_GROUP_HIDDEN;
  }else{
   if(name.starts_with("anim_face"))t.mImageOverride=SandboxArt::Image(family,triple?(closed?"small-blink":"small-head"):(closed?"blink":"head"));
   else if(name.find("mouth")!=name.npos&&def->base!=40)t.mImageOverride=SandboxArt::Image(def->element==Element::Alternating?"fire":family,triple?"small-mouth":"mouth");
   // Gatling's mouth, barrel and overlay are separate original bones. All three keep native dimensions.
   else if(def->base==40&&name=="GatlingPea_mouth")t.mImageOverride=SandboxArt::Image(family,"gatling-mouth");
   else if(def->base==40&&name.starts_with("GatlingPea_barrel"))t.mImageOverride=SandboxArt::Image(family,"gatling-barrel");
   else if(def->base==40&&name=="GatlingPea_mouth_overlay")t.mImageOverride=SandboxArt::Image(family,"gatling-overlay");
   else if(name.find("blink")!=name.npos)t.mRenderGroup=RENDER_GROUP_HIDDEN;
  }
 }
}
void SkinPlant(Plant* plant,State& s){
 for(auto id:{plant->mBodyReanimID,plant->mHeadReanimID,plant->mHeadReanimID2,plant->mHeadReanimID3})
  Skin(gLawnApp->ReanimationTryToGet(id),s.id,s.closed,plant->mPlantHealth);
}
Sexy::MemoryImage* Card(int id){
 auto& cached=cards[id];if(cached)return cached.get();
 const auto* d=Find(id);const auto seed=static_cast<SeedType>(d->base);
 cached=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(180,180);
 Sexy::Graphics g(cached.get());g.SetLinearBlend(true);
 auto frame=[&](const char* layer){
  Reanimation a;a.ReanimationInitializeType(40,40,GetPlantDefinition(seed).mReanimationType);
  if(!a.TrackExists(layer))return;a.SetFramesForLayer(layer);if(d->base==1)a.mAnimTime=0.15f;
  Skin(&a,id,false);a.Draw(&g);
 };
 frame("anim_idle");
 if(d->base==18){frame("anim_head_idle1");frame("anim_head_idle3");frame("anim_head_idle2");}
 else if(d->base==0||d->base==7||d->base==40)frame("anim_head_idle");
 return cached.get();
}
void Pulse(Board* b,Plant* p){
 const float x=p->mX+50,y=p->mY+35;
 beams.push_back({x,y,780,y,18,false});
 for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mPosX+65>x&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY)))z->TakeDamage(18,0);
}
}
void Reset(){states.clear();shots.clear();poison.clear();beams.clear();}
void Forget(Plant* p){states.erase(p);}
void ForgetShot(Projectile* p){shots.erase(p);}
bool IsCustom(const Plant* p){return gSandboxEnabled&&states.contains(p);}
int Type(const Plant* p){auto it=states.find(p);return it==states.end()?int(p->mSeedType):it->second.id;}
void Assign(Plant* p,int id){
 auto* d=Find(id);if(!d)return;
 auto& s=states[p];s={id};s.health=p->mPlantHealth;
 if(id==109)p->mPlantHealth=p->mPlantMaxHealth=s.health=2500;
 if(d->rate){p->mLaunchRate=d->rate;p->mLaunchCounter=std::min(p->mLaunchCounter,d->rate);}
 if(id==111||id==108)p->mLaunchCounter=9999;
 SkinPlant(p,s);
}
void AdjustScale(const Plant* p,float& x,float& y,float& sx,float& sy){
 if(!gSandboxEnabled)return;auto* d=Find(Type(p));if(!d||d->scale==1)return;
 x+=40*sx*(1-d->scale);y+=80*sy*(1-d->scale);sx*=d->scale;sy*=d->scale;
}
int NextShot(Plant* p){
 if(!gSandboxEnabled)return 0;auto it=states.find(p);if(it==states.end())return 0;
 const auto e=ShotElement(it->second.id,it->second.shots++);return e==Element::Ice?1:e==Element::Fire?2:0;
}
void OnFired(Plant* p,Projectile* shot,Zombie* target){
 if(!gSandboxEnabled)return;auto* d=Find(Type(p));if(!d||d->id<112)return;
 shots[shot]={d->id,d->damage};
 if(d->scale!=1){shot->mPosX=p->mX+40+(shot->mPosX-p->mX-40)*d->scale;shot->mPosY=p->mY+80+(shot->mPosY-p->mY-80)*d->scale;shot->mX=int(shot->mPosX);shot->mY=int(shot->mPosY);}
 if(d->id==116&&target){shot->mMotionType=MOTION_HOMING;shot->mTargetZombieID=p->mBoard->ZombieGetID(target);shot->mVelX=3.0f;}
 if(d->id==114){shot->mMotionType=MOTION_STAR;shot->mVelX=2.2f;shot->mVelY=0;}
 if(d->id==115){
  for(int dy:{-1,1}){
   const int row=p->mRow+dy;if(row<0||row>=(p->mBoard->StageHasPool()?6:5))continue;
   auto* extra=p->mBoard->AddProjectile(shot->mPosX,shot->mPosY,shot->mRenderOrder,row,PROJECTILE_PEA);
   extra->mDamageRangeFlags=shot->mDamageRangeFlags;extra->mMotionType=MOTION_THREEPEATER;extra->mVelY=dy*3.0f;extra->mShadowY-=dy*80.0f;
   shots[extra]={115,d->damage};
  }
 }
}
void UpdateShot(Projectile* p){
 auto it=shots.find(p);if(it==shots.end()||it->second.id!=116||p->mDead)return;
 auto* current=p->mBoard->ZombieTryToGet(p->mTargetZombieID);
 if(current&&Enemy(current))return;
 Zombie* best=nullptr;float distance=100000;
 for(auto* z:p->mBoard->mZombies)if(Enemy(z)&&z->EffectedByDamage(p->mDamageRangeFlags)){
  const float d=std::abs(z->mPosX-p->mPosX)+std::abs(z->mPosY-p->mPosY);
  if(d<distance){distance=d;best=z;}
 }
 if(best)p->mTargetZombieID=p->mBoard->ZombieGetID(best);
 else {p->mMotionType=MOTION_STRAIGHT;p->mVelX=3.3f;p->mVelY=0;}
}
bool Impact(Projectile* p,Zombie* target){
 auto it=shots.find(p);if(it==shots.end())return false;
 const auto shot=it->second;if(!target)return true;
 if(shot.id==112){
  std::vector<ZombieID> hit;Zombie* from=target;int damage=shot.damage;
  for(int hop=0;hop<3&&from;++hop){
   hit.push_back(p->mBoard->ZombieGetID(from));
   const float x=from->mPosX+55,y=from->mPosY+55;
   if(!SandboxZombies::ElectricHit(from))from->TakeDamage(damage,p->GetDamageFlags(from));
   Zombie* next=nullptr;float distance=150;
   for(auto* z:p->mBoard->mZombies)if(Enemy(z)&&z->EffectedByDamage(p->mDamageRangeFlags)&&std::find(hit.begin(),hit.end(),p->mBoard->ZombieGetID(z))==hit.end()){
    float d=std::hypot(z->mPosX+55-x,z->mPosY+55-y);if(d<distance){distance=d;next=z;}
   }
   if(next&&hop<2)beams.push_back({x,y,next->mPosX+55,next->mPosY+55,16,true});
   from=next;damage=std::max(5,damage-6);
  }
 }else{
  target->TakeDamage(shot.damage,p->GetDamageFlags(target));
  if(shot.id==114&&Enemy(target)&&target->mZombieType!=ZOMBIE_GARGANTUAR&&target->mZombieType!=ZOMBIE_REDEYE_GARGANTUAR&&target->mZombieType!=ZOMBIE_ZAMBONI){
   target->mPosX=std::min(850.0f,target->mPosX+35);target->UpdateReanim();
  }
  if(shot.id==117&&Enemy(target))poison[p->mBoard->ZombieGetID(target)]=SandboxCombatRules::PoisonDuration;
 }
 return true;
}
void Tick(Board* b){
 if(b->mPaused)return;
 for(auto it=beams.begin();it!=beams.end();)if(--it->ticks<=0)it=beams.erase(it);else ++it;
 for(auto it=poison.begin();it!=poison.end();){
  auto* z=b->ZombieTryToGet(it->first);
  if(!z||!Enemy(z)||it->second<=0){it=poison.erase(it);continue;}
  if(SandboxCombatRules::PoisonPulse(it->second))z->TakeDamage(SandboxCombatRules::PoisonDamage,0);
  --it->second;++it;
 }
 for(auto* p:b->mPlants){
  if(p->mDead){Forget(p);continue;}auto it=states.find(p);if(it==states.end())continue;auto& s=it->second;
  const bool closed=p->mBlinkCountdown>0&&p->mBlinkCountdown<=8&&p->mShootingCounter==0;
  if(closed!=s.closed||s.health!=p->mPlantHealth){s.closed=closed;SkinPlant(p,s);}
  const bool hurt=p->mPlantHealth<s.health;s.health=p->mPlantHealth;if(s.cooldown>0)--s.cooldown;
  if(p->mIsAsleep)continue;++s.age;
  if(s.id==108){
   p->mLaunchCounter=9999;
   if(s.echo>=0){if(SandboxCombatRules::EchoPulse(s.echo))Pulse(b,p);--s.echo;}
   if(s.echo<0&&s.age%240==0&&p->FindTargetZombie(p->mRow,WEAPON_PRIMARY))s.echo=SandboxCombatRules::EchoDelay;
  }
  if(s.id==109&&hurt&&s.cooldown==0){
   for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&std::abs(z->mPosX+45-(p->mX+40))<95){
    z->mPosX=std::min(850.0f,z->mPosX+70);z->UpdateReanim();
   }s.cooldown=SandboxCombatRules::SpringCooldown;
  }
  if(s.id==111)p->mLaunchCounter=9999;
 }
 // Auras are evaluated once per recipient, so surrounding it with flowers cannot stack infinitely.
 for(auto* p:b->mPlants)if(!p->mDead&&!p->mIsAsleep&&p->mLaunchCounter>1&&GetPlantDefinition(p->mSeedType).mSubClass==SUBCLASS_SHOOTER){
  bool buff=false;
  for(const auto& [source,s]:states)if(s.id==110&&!source->mDead&&!source->mIsAsleep&&std::abs(source->mRow-p->mRow)<=1&&std::abs(source->mPlantCol-p->mPlantCol)<=1&&source!=p){buff=true;break;}
  if(buff&&b->mMainCounter%2==0&&SandboxCombatRules::CanHaste(p->mLaunchCounter))--p->mLaunchCounter;
 }
 // Pair with the nearest active mushroom below; a zombie takes at most one pulse even at a junction.
 for(auto* z:b->mZombies)if(Enemy(z)&&b->mMainCounter%25==0){
  bool hit=false;
  for(const auto& [a,sa]:states)if(sa.id==111&&!a->mDead&&!a->mIsAsleep){
   const Plant* bottom=nullptr;
   for(const auto& [c,sc]:states)if(sc.id==111&&!c->mDead&&!c->mIsAsleep&&SandboxCombatRules::Connected(a->mPlantCol,a->mRow,c->mPlantCol,c->mRow)&&(!bottom||c->mRow<bottom->mRow))bottom=c;
   if(bottom&&z->mRow>=a->mRow&&z->mRow<=bottom->mRow&&z->EffectedByDamage(const_cast<Plant*>(a)->GetDamageRangeFlags(WEAPON_PRIMARY))&&std::abs(z->mPosX+55-(a->mX+40))<24){hit=true;break;}
  }
  if(hit&&!SandboxZombies::ElectricHit(z))z->TakeDamage(12,0);
 }
}
void DrawEffects(Sexy::Graphics* g,Board* b){
 for(const auto& beam:beams){
  g->SetColor(beam.electric?Sexy::Color(110,205,255,210):Sexy::Color(221,204,151,200));
  g->DrawLine(beam.x1,beam.y1,beam.x2,beam.y2);g->DrawLine(beam.x1,beam.y1+2,beam.x2,beam.y2+2);
 }
 for(const auto& [p,s]:states)if(!p->mDead&&!p->mIsAsleep&&s.id==111){
  const Plant* bottom=nullptr;
  for(const auto& [c,cs]:states)if(cs.id==111&&!c->mDead&&!c->mIsAsleep&&SandboxCombatRules::Connected(p->mPlantCol,p->mRow,c->mPlantCol,c->mRow)&&(!bottom||c->mRow<bottom->mRow))bottom=c;
  if(bottom){int x=p->mX+40,y=p->mY+25,end=bottom->mY+25;g->SetColor(Sexy::Color(133,208,240,190));
   for(int n=y;n<end;n+=12){int mid=std::min(end,n+6),last=std::min(end,n+12);int offset=(n/12%2?4:-4);g->DrawLine(x,n,x+offset,mid);g->DrawLine(x+offset,mid,x,last);}
  }
 }
 for(auto [id,time]:poison)if(auto* z=b->ZombieTryToGet(id);z&&Enemy(z)){
  g->SetColor(Sexy::Color(135,170,66,170));g->FillRect(int(z->mPosX+45),int(z->mPosY+26),3,3);
 }
}
void DrawShot(Sexy::Graphics* g,const Projectile* p){
 const auto it=shots.find(p);if(it==shots.end())return;
 // A small trailing glint preserves the original pea sprite and collision size.
 const auto id=it->second.id;
 g->SetColor(id==112?Sexy::Color(96,175,244,190):id==117?Sexy::Color(139,75,166,170):Sexy::Color(225,217,146,140));
 if(id==112||id==116||id==117)g->DrawLine(-8,5,0,5);
}
void DrawCard(Sexy::Graphics* g,int x,int y,int id){
 const auto* d=Find(id);
 if(!d){DrawSeedPacket(g,x,y,static_cast<SeedType>(id),SEED_NONE,0,255,false,false);return;}
 PvzpDrawImageCelScaledF(g,Sexy::IMAGE_SEEDS,x,y,2,0,1,1);
 Sexy::Graphics clip(*g);clip.SetClipRect(x+3,y+9,44,48);
 SandboxArt::DrawFit(&clip,Card(id),x+4,y+10,42,44,d->id==113?0.82f:1.0f);
 PvzpDrawString(g,"0",x+25,y+65,Sexy::FONT_BRIANNETOD12,Sexy::Color(55,64,23),DS_ALIGN_CENTER);
}
}
