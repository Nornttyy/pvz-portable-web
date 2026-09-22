// Original sandbox roster; skins and combat state are per instance and never saved into adventure.
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "SandboxArt.h"
#include "SandboxCombatRules.h"
#include "SandboxVisualRules.h"
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
struct State { int id,shots=0,age=0,cooldown=0,health=0,echo=-1,stage=0,charge=0;bool closed=false; };
struct Shot { int id,damage,hops=3;float limit=10000; };
struct Beam { float x1,y1,x2,y2;int ticks;bool electric;int row; };
struct Effect {float x,y;int row,art,ticks=18;bool muzzle=false;};
std::map<const Plant*,State> states;
std::map<const Projectile*,Shot> shots;
std::map<ZombieID,int> poison;
std::map<int,std::unique_ptr<Sexy::MemoryImage>> cards;
std::vector<Beam> beams;
std::vector<Effect> effects;
void EffectAt(float x,float y,int row,int art,bool muzzle=false){
 if(effects.size()>=256)effects.erase(effects.begin());
 effects.push_back({x,y,row,art,muzzle?10:18,muzzle});
}
int Art(const Projectile* p,int id){return SandboxVisualRules::ArtIndex(id,p->mProjectileType==PROJECTILE_FIREBALL,p->mProjectileType==PROJECTILE_SNOWPEA);}
bool Muzzle(const Plant* p,int row,float& x,float& y){
 const auto* d=Find(Type(p));if(!d)return false;
 auto reanim=p->mHeadReanimID;const char* track="idle_mouth";float w=35,h=49;
 if(d->base==18){
  const int head=SandboxVisualRules::HeadForRow(p->mRow,row);
  reanim=head==1?p->mHeadReanimID:head==2?p->mHeadReanimID2:p->mHeadReanimID3;
  track=head==1?"ThreePeater_mouth1":head==2?"ThreePeater_mouth2":"ThreePeater_mouth3";w=19;h=43;
 }else if(d->base==40){track="GatlingPea_barrel3";w=43;h=27;} // Front barrel, not the rear green housing.
 float localX,localY;
 if(!SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(reanim),track,w,h,w-3,h*0.5f,localX,localY))return false;
 x=p->mX+localX;y=p->mY+localY;return true;
}
void PlantPoint(const Plant* p,const char* track,float w,float h,float& x,float& y){
 float px,py;if(SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(p->mBodyReanimID),track,w,h,w*0.5f,h*0.5f,px,py)){x=p->mX+px;y=p->mY+py;}
}
bool Enemy(Zombie* z){return !z->mMindControlled&&!z->IsDeadOrDying()&&z->mHasHead;}
void Skin(Reanimation* anim,int id,bool closed,int health=4000,int stage=0){
 if(!anim)return;
 const auto* def=Find(id);if(!def)return;
 const bool triple=def->base==18;
 const char* family=def->art?def->art:(def->element==Element::Fire?"fire":"ice");
 if(id==111)family=stage==2?"storm-mushroom-2":stage==1?"storm-mushroom-1":"storm-mushroom-0";
 for(int i=0;i<anim->mDefinition->mTracks.count;++i){
  const std::string_view name=anim->mDefinition->mTracks.tracks[i].mName;
  auto& t=anim->mTrackInstances[i];
  if(id==110){
   if(name=="anim_idle")t.mImageOverride=SandboxArt::Image(family,closed?"blink":"head");
   if(name.find("blink")!=name.npos)t.mRenderGroup=RENDER_GROUP_HIDDEN;
  }else if(id==111){
   if(name=="anim_face")t.mImageOverride=SandboxArt::Image(family,closed?"blink":"head");
   if(name=="PuffShroom_head")t.mImageOverride=SandboxArt::Image(family,"cap");
   if(name=="PuffShroom_stem")t.mImageOverride=SandboxArt::Image(family,"stem");
   if(name=="PuffShroom_eyes"||name.find("blink")!=name.npos)t.mRenderGroup=RENDER_GROUP_HIDDEN;
  }else{
   if(name=="anim_face"||name=="anim_face1"||name=="anim_face2"||name=="anim_face3")
    t.mImageOverride=SandboxArt::Image(family,def->base==40?(closed?"gatling-blink":"gatling-head"):triple?(closed?"small-blink":"small-head"):(closed?"blink":"head"));
   else if(name=="idle_mouth"||name=="ThreePeater_mouth1"||name=="ThreePeater_mouth2"||name=="ThreePeater_mouth3")
    t.mImageOverride=SandboxArt::Image(def->element==Element::Alternating?"fire":family,triple?"small-mouth":"mouth");
   // Keep all original Gatling hardware and its native painter order. The rear
   // housing, four rotating barrels and front occluder are NOT interchangeable.
   else if(def->base==40&&(name=="GatlingPea_mouth"||name.starts_with("GatlingPea_barrel")||name=="GatlingPea_mouth_overlay"))
    t.mImageOverride=nullptr;
   else if(name.find("blink")!=name.npos)t.mRenderGroup=RENDER_GROUP_HIDDEN;
  }
 }
}
void SkinPlant(Plant* plant,State& s){
 for(auto id:{plant->mBodyReanimID,plant->mHeadReanimID,plant->mHeadReanimID2,plant->mHeadReanimID3})
  Skin(gLawnApp->ReanimationTryToGet(id),s.id,s.closed,plant->mPlantHealth,s.stage);
}
Sexy::MemoryImage* Card(int id){
 auto& cached=cards[id];if(cached)return cached.get();
 const auto* d=Find(id);const auto seed=static_cast<SeedType>(d->base);
 cached=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(180,180);
 Sexy::Graphics g(cached.get());g.SetLinearBlend(true);
 auto frame=[&](const char* layer){
  Reanimation a;a.ReanimationInitializeType(40,40,GetPlantDefinition(seed).mReanimationType);
  if(!a.TrackExists(layer))return;a.SetFramesForLayer(layer);if(d->base==1)a.mAnimTime=0.15f;
  Skin(&a,id,false,4000,id==111?2:0);a.Draw(&g);
 };
 frame("anim_idle");
 if(d->base==18){frame("anim_head_idle1");frame("anim_head_idle3");frame("anim_head_idle2");}
 else if(d->base==0||d->base==7||d->base==40)frame("anim_head_idle");
 return cached.get();
}
void Pulse(Board* b,Plant* p){
 float x=p->mX+50,y=p->mY+35;Muzzle(p,p->mRow,x,y);
 beams.push_back({x,y,780,y,36,false,p->mRow});
 for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mPosX+65>x&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY)))z->TakeDamage(18,0);
}
}
void Reset(){states.clear();shots.clear();poison.clear();beams.clear();effects.clear();}
void Forget(Plant* p){states.erase(p);SandboxZombies::ForgetPlant(p);}
void ForgetShot(Projectile* p){shots.erase(p);}
bool IsCustom(const Plant* p){return gSandboxEnabled&&states.contains(p);}
int Type(const Plant* p){auto it=states.find(p);return it==states.end()?int(p->mSeedType):it->second.id;}
int GrowthStage(const Plant* p){auto it=states.find(p);return it==states.end()?0:it->second.stage;}
void Assign(Plant* p,int id){
 auto* d=Find(id);if(!d)return;
 auto& s=states[p];s={id};s.health=p->mPlantHealth;
 if(id==111)s.cooldown=80;
 if(d->rate){p->mLaunchRate=d->rate;p->mLaunchCounter=std::min(p->mLaunchCounter,d->rate);}
 if(id==111||id==108)p->mLaunchCounter=9999;
 SkinPlant(p,s);
}
void AdjustScale(const Plant* p,float& x,float& y,float& sx,float& sy){
 if(!gSandboxEnabled)return;auto* d=Find(Type(p));if(!d)return;
 const float scale=d->id==111?1.0f+GrowthStage(p)*0.18f:d->scale;
 x+=SandboxVisualRules::GroundX*sx*(1-scale);y+=SandboxVisualRules::GroundY*sy*(1-scale);sx*=scale;sy*=scale;
}
void AdjustShadow(const Plant* p,float& x,float& y,float& scale){
 if(!gSandboxEnabled)return;auto* d=Find(Type(p));if(!d)return;
 // Native center-scaled shadow keeps its center while its width follows the body.
 scale*=d->id==111?1.0f+GrowthStage(p)*0.18f:d->scale;
}
float ShotScale(const Projectile* p){auto i=shots.find(p);return i==shots.end()?1.0f:i->second.id==113?0.55f:i->second.id==115?0.7f:i->second.id==114?1.1f:1.0f;}
bool HasShot(const Projectile* p){return shots.contains(p);}
int ShotRadius(const Projectile* p){auto it=shots.find(p);return it==shots.end()?12:std::max(5,int(SandboxVisualRules::Shots[Art(p,it->second.id)].h*0.45f));}
int NextShot(Plant* p){
 if(!gSandboxEnabled)return 0;auto it=states.find(p);if(it==states.end())return 0;
 const auto e=ShotElement(it->second.id,it->second.shots++);return e==Element::Ice?1:e==Element::Fire?2:0;
}
void OnFired(Plant* p,Projectile* shot,Zombie* target){
 if(!gSandboxEnabled)return;auto* d=Find(Type(p));if(!d)return;
 shots[shot]={d->id,d->damage};
 float mx,my;
 if(Muzzle(p,shot->mRow,mx,my)){
  shot->mPosX=mx-SandboxVisualRules::PeaCenter;
  shot->mPosY=my-SandboxVisualRules::PeaCenter-shot->mPosZ;
 }else if(d->scale!=1){
  shot->mPosX=p->mX+SandboxVisualRules::GroundX+(shot->mPosX-p->mX-SandboxVisualRules::GroundX)*d->scale;
  shot->mPosY=p->mY+SandboxVisualRules::GroundY+(shot->mPosY-p->mY-SandboxVisualRules::GroundY)*d->scale;
 }
 shot->mX=int(shot->mPosX);shot->mY=int(shot->mPosY+shot->mPosZ);
 EffectAt(shot->mPosX+12,shot->mPosY+shot->mPosZ+12,p->mRow,Art(shot,d->id),true);
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
 auto it=shots.find(p);if(it==shots.end()||p->mDead)return;
 if(it->second.id==111&&p->mPosX>it->second.limit){p->Die();return;}
 if(it->second.id!=116)return;
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
 const auto shot=it->second;
 EffectAt(p->mPosX+12,p->mPosY+p->mPosZ+12,target?target->mRow:p->mRow,Art(p,shot.id));
 // The original eight keep their native fire splash / ice slow combat, only their artwork changes.
 if(shot.id<112&&shot.id!=111)return false;
 if(!target)return true;
 if(shot.id==112||shot.id==111){
  std::vector<ZombieID> hit;Zombie* from=target;int damage=shot.damage;
  for(int hop=0;hop<shot.hops&&from;++hop){
   hit.push_back(p->mBoard->ZombieGetID(from));
   const float x=from->mPosX+55,y=from->mPosY+55;
   if(!SandboxZombies::ElectricHit(from))from->TakeDamage(damage,p->GetDamageFlags(from));
   Zombie* next=nullptr;float distance=150;
   for(auto* z:p->mBoard->mZombies)if(Enemy(z)&&z->EffectedByDamage(p->mDamageRangeFlags)&&std::find(hit.begin(),hit.end(),p->mBoard->ZombieGetID(z))==hit.end()){
    float d=std::hypot(z->mPosX+55-x,z->mPosY+55-y);if(d<distance){distance=d;next=z;}
   }
   if(next&&hop+1<shot.hops)beams.push_back({x,y,next->mPosX+55,next->mPosY+55,16,true,from->mRow});
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
 for(auto it=effects.begin();it!=effects.end();)if(--it->ticks<=0)it=effects.erase(it);else ++it;
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
  s.health=p->mPlantHealth;
  if(p->mIsAsleep)continue;++s.age;
  const bool inkDelay=SandboxZombies::AttackSlowed(p)&&b->mMainCounter%3==0;
  if(s.cooldown>0&&!inkDelay)--s.cooldown;
  if(s.id==108){
   p->mLaunchCounter=9999;
   if(s.echo>=0){if(SandboxCombatRules::EchoPulse(s.echo))Pulse(b,p);--s.echo;}
   if(s.echo<0&&s.age%240==0&&p->FindTargetZombie(p->mRow,WEAPON_PRIMARY))s.echo=SandboxCombatRules::EchoDelay;
  }
  if(s.id==111){
   p->mLaunchCounter=9999;
   const int stage=SandboxCombatRules::GrowthStage(s.age);
   if(stage!=s.stage){s.stage=stage;SkinPlant(p,s);EffectAt(p->mX+40,p->mY+30,p->mRow,2);}
   Zombie* target=nullptr;float distance=SandboxCombatRules::GrowthRange(stage);
   for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))){
    const float d=z->mPosX+55-(p->mX+40);if(d>=0&&d<distance){distance=d;target=z;}
   }
   if(!target){s.charge=0;continue;}
   if(s.cooldown==0&&s.charge==0)s.charge=25;
   if(s.charge>0&&!inkDelay&&--s.charge==0){
    if(b->mProjectiles.mSize>=b->mProjectiles.mMaxSize-8){s.cooldown=30;continue;}
    float x=p->mX+40,y=p->mY+20;PlantPoint(p,"PuffShroom_head",81,53,x,y);
    auto* shot=b->AddProjectile(x-12,y-12,p->mRenderOrder,p->mRow,PROJECTILE_PEA);
    if(shot){
     shot->mDamageRangeFlags=p->GetDamageRangeFlags(WEAPON_PRIMARY);
     shots[shot]={111,SandboxCombatRules::GrowthDamage(stage),stage==2?3:1,float(p->mX+40+SandboxCombatRules::GrowthRange(stage))};
     EffectAt(x,y,p->mRow,2,true);
    }
    s.cooldown=SandboxCombatRules::GrowthRate(stage);
   }
  }
 }
 // Auras are evaluated once per recipient, so surrounding it with flowers cannot stack infinitely.
 for(auto* p:b->mPlants)if(!p->mDead&&!p->mIsAsleep&&p->mLaunchCounter>1&&GetPlantDefinition(p->mSeedType).mSubClass==SUBCLASS_SHOOTER){
  bool buff=false;
  for(const auto& [source,s]:states)if(s.id==110&&!source->mDead&&!source->mIsAsleep&&std::abs(source->mRow-p->mRow)<=1&&std::abs(source->mPlantCol-p->mPlantCol)<=1&&source!=p){buff=true;break;}
  if(buff&&b->mMainCounter%2==0&&SandboxCombatRules::CanHaste(p->mLaunchCounter))--p->mLaunchCounter;
 }
}
void DrawEffects(Sexy::Graphics* graphics,Board* b,int row){
 Sexy::Graphics clipped(*graphics);clipped.ClipRect(0,82,800,518);auto* g=&clipped;
 for(const auto& beam:beams)if(beam.row==row){
  if(beam.electric)SandboxArt::Link(g,beam.x1,beam.y1,beam.x2,beam.y2,b->mMainCounter/4,std::min(230,beam.ticks*25));
  else {const float t=1-beam.ticks/36.0f,x=beam.x1+(beam.x2-beam.x1)*t;
   SandboxArt::Sprite(g,(b->mMainCounter/5)%2?"sonic-1":"sonic-0",x,beam.y1,20,42,0,std::min(220,beam.ticks*25));
  }
 }
 for(const auto& e:effects)if(e.row==row){
  const int age=(e.muzzle?10:18)-e.ticks;const float grow=e.muzzle?1:0.7f+age/36.0f;
  if(e.art==8)SandboxArt::Sprite(g,age<8?"spring-0":"spring-1",e.x,e.y,44*grow,27*grow,0,std::min(230,e.ticks*30));
  else {const auto& a=SandboxVisualRules::Shots[e.art];
   const char* file=e.muzzle?(e.art==1||e.art==2?"muzzle-ice":"muzzle-warm"):age<8?a.hit0:a.hit1;
   const float size=e.muzzle?(e.art==3?11:18):a.impact*grow;
   SandboxArt::Sprite(g,file,e.x,e.y,size,size,0,std::min(240,e.ticks*35));
  }
 }
 for(const auto& [p,s]:states)if(!p->mDead&&!p->mIsAsleep&&p->mRow==row&&s.id==111&&s.charge>0){
  float x=p->mX+40,y=p->mY+20;PlantPoint(p,"PuffShroom_head",81,53,x,y);
  const float size=12+(25-s.charge)*0.7f;
  SandboxArt::Sprite(g,"charge",x,y,size,size,0,190);
 }
 for(const auto& [p,s]:states)if(s.id==110&&!p->mDead&&!p->mIsAsleep&&p->mRow==row&&s.age%100<45){
  float x=p->mX+40,y=p->mY+20;PlantPoint(p,"anim_idle",57,43,x,y);
  SandboxArt::Sprite(g,"notes",x+24,y-15-(s.age%100)*0.3f,15,19,0,160);
 }
 for(auto [id,time]:poison)if(auto* z=b->ZombieTryToGet(id);z&&Enemy(z)&&z->mRow==row)
  SandboxArt::Sprite(g,"poison",z->mPosX+42,z->mPosY+40-(b->mMainCounter%40)*0.25f,15,20,0,170);
}
bool DrawShot(Sexy::Graphics* g,const Projectile* p){
 const auto it=shots.find(p);if(it==shots.end())return false;
 const auto& a=SandboxVisualRules::Shots[Art(p,it->second.id)];
 const float angle=p->mMotionType==MOTION_HOMING?std::atan2(p->mVelY,p->mVelX):0;
 const float ox=a.w*0.5f-a.coreX,oy=a.h*0.5f-a.coreY,c=std::cos(angle),s=std::sin(angle);
 // Core center stays at the native pea's collision center even when the drawn tail is asymmetric.
 SandboxArt::Sprite(g,a.sprite,p->mPosX-p->mX+12+c*ox-s*oy,p->mPosY+p->mPosZ-p->mY+12+s*ox+c*oy,a.w,a.h,angle);
 return true;
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
