// Native plants infused with meme powers. Old standalone originals are retired.
#include "SandboxPlants.h"
#include "SandboxArt.h"
#include "SandboxMemeRules.h"
#include "SandboxVisualRules.h"
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
#include <set>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <cmath>
#include <algorithm>
namespace SandboxPlants {
namespace {
struct State {int id=0,health=0,empowered=32,remaining=0,spacing=0;SandboxMemeRules::State heat;};
std::map<const Plant*,State> states;
std::set<const Projectile*> shots;
std::map<const Projectile*,int> damageShots;
std::map<int,std::unique_ptr<Sexy::MemoryImage>> cards;
constexpr int DamageStage(int h,int m){return m<=0?2:h*3<=m?2:h*3<=m*2?1:0;}
// Temporary per-instance overrides: never leave a tinted walnut image installed
// during AnimateNuts(), which compares native image pointers to detect damage.
struct WarmSkin {
 std::vector<std::pair<ReanimatorTrackInstance*,Sexy::Image*>> previous;
 void Apply(Reanimation* anim,int id,int level,int health,int maximum,bool blink=false){
  if(!anim)return;
  for(int i=0;i<anim->mDefinition->mTracks.count;++i){
   const std::string_view name=anim->mDefinition->mTracks.tracks[i].mName;
   const char* file=nullptr;
   const int base=Base(id),power=SandboxMemeRules::PowerOf(id);
   if(base!=1&&base!=3&&base!=23){
    if(!blink&&(name.starts_with("anim_face")||name=="GatlingPea_head"))file="PeaShooter_Head.png";
    if(!blink&&(name=="idle_mouth"||name=="SnowPea_mouth"||name.starts_with("ThreePeater_mouth")))file="PeaShooter_mouth.png";
    if(name=="idle_shoot_blink"||name.starts_with("anim_blink")||(name.starts_with("ThreePeater_head")&&name.ends_with("_blink")))file=anim->mAnimTime<0.5f?"PeaShooter_blink1.png":"PeaShooter_blink2.png";
   }else if(base==1){
    if(!blink&&name=="anim_idle")file="SunFlower_head.png";
    if(name=="anim_blink")file=anim->mAnimTime<0.5f?"SunFlower_blink1.png":"SunFlower_blink2.png";
   }else if(base==3||base==23){
    if(!blink&&(name=="anim_face"||(base==23&&name=="anim_idle"))){
     const int damage=DamageStage(health,maximum);
     file=damage==2?"Wallnut_cracked2.png":damage==1?"Wallnut_cracked1.png":"Wallnut_body.png";
    }
    if(name.starts_with("anim_blink"))file=anim->mAnimTime<0.5f?"Wallnut_blink1.png":"Wallnut_blink2.png";
   }
   if(!file)continue;
   std::string native=file;
   if(base==5){if(native=="PeaShooter_Head.png")native="SnowPea_head.png";else if(native.starts_with("PeaShooter_"))native="SnowPea_"+native.substr(11);}
   if(base==18){if(native=="PeaShooter_Head.png")native="ThreePeater_head.png";else if(native.starts_with("PeaShooter_"))native="ThreePeater_"+native.substr(11);}
   if(base==40){if(native=="PeaShooter_Head.png")native="GatlingPea_head.png";else if(native.starts_with("PeaShooter_"))native="GatlingPea_"+native.substr(11);}
   if(base==23&&native.starts_with("Wallnut_"))native="Tallnut_"+native.substr(8);
   auto& track=anim->mTrackInstances[i];
   previous.emplace_back(&track,track.mImageOverride);track.mImageOverride=SandboxArt::PowerNative(native.c_str(),level,power);
  }
 }
 ~WarmSkin(){for(auto it=previous.rbegin();it!=previous.rend();++it)it->first->mImageOverride=it->second;}
};
void NativePuff(Sexy::Graphics* g,float x,float y,float size,int age,int alpha){
 auto* image=SandboxArt::NativeImage(age<15?"puff_3.png":"puff_4.png");if(!image)return;
 Sexy::SexyTransform2D m;m.LoadIdentity();m.m00=size/image->mWidth;m.m11=size/image->mHeight;
 m.m02=x+g->mTransX;m.m12=y+g->mTransY;
 PvzpBltMatrix(g,image,m,g->mClipRect,Sexy::Color(255,255,255,alpha),g->mDrawMode,Sexy::Rect(0,0,image->mWidth,image->mHeight));
}
void HeatBrow(Sexy::Graphics* g,Reanimation* head,int level,int alpha){
 if(!head||!head->TrackExists("anim_face")||alpha<=0)return;
 auto* face=SandboxArt::NativeImage("PeaShooter_Head.png");auto* brow=SandboxArt::WarmNative("PeaShooter_eyebrow.png",level);if(!face||!brow)return;
 Sexy::SexyTransform2D m;head->GetTrackMatrix(head->FindTrackIndex("anim_face"),m);
 // Registration taken from frame 29 of the original Repeater reanimation.
 // This keeps both eyebrows on the original eye positions, including recoil.
 const float x=(34.8f+0.8f*brow->mWidth/2-(19.2f+0.555f*face->mWidth/2))/0.555f;
 const float y=(19.7f+0.8f*brow->mHeight/2-(17.8f+0.5f*face->mHeight/2))/0.5f;
 m.m02+=m.m00*x+m.m01*y+g->mTransX;m.m12+=m.m10*x+m.m11*y+g->mTransY;
 m.m00*=0.8f/0.555f;m.m10*=0.8f/0.555f;m.m01*=1.6f;m.m11*=1.6f;
 PvzpBltMatrix(g,brow,m,g->mClipRect,Sexy::Color(255,255,255,alpha),g->mDrawMode,Sexy::Rect(0,0,brow->mWidth,brow->mHeight));
}

bool Enemy(Zombie* z){return !z->mMindControlled&&!z->IsDeadOrDying()&&z->mHasHead;}
Sexy::MemoryImage* Card(int id){
 auto& cached=cards[id];if(cached)return cached.get();
 const auto seed=static_cast<SeedType>(Base(id));
 cached=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(180,180);
 Sexy::Graphics g(cached.get());g.SetLinearBlend(true);
 auto frame=[&](const char* layer){
  Reanimation a;a.ReanimationInitializeType(40,40,GetPlantDefinition(seed).mReanimationType);
  if(!a.TrackExists(layer))return;a.SetFramesForLayer(layer);if(seed==1)a.mAnimTime=0.15f;
  WarmSkin warm;warm.Apply(&a,id,24,4000,4000);a.Draw(&g);
  if(Base(id)==0&&SandboxMemeRules::PowerOf(id)==180&&std::string_view(layer)=="anim_head_idle")HeatBrow(&g,&a,24,255);
 };
 frame("anim_idle");
 if(seed==18){frame("anim_head_idle1");frame("anim_head_idle3");frame("anim_head_idle2");}
 else if(seed==0||seed==5||seed==7||seed==40)frame("anim_head_idle");
 return cached.get();
}
}
void Reset(){states.clear();shots.clear();damageShots.clear();MemeCharacters::Reset();}
void Forget(Plant* p){states.erase(p);MemeCharacters::Forget(p);}
void ForgetShot(Projectile* p){shots.erase(p);damageShots.erase(p);MemeCharacters::ForgetShot(p);}
bool IsCustom(const Plant* p){return states.contains(p)||MemeCharacters::Is(p);}
int Type(const Plant* p){if(MemeCharacters::Is(p))return MemeCharacters::Type(p);auto it=states.find(p);return it==states.end()?int(p->mSeedType):it->second.id;}
int GrowthStage(const Plant* p){auto it=states.find(p);return it==states.end()?0:it->second.heat.heat/250;}
int HeatData(const Plant* p,int field){if(MemeCharacters::Is(p))return MemeCharacters::Data(p,field);auto it=states.find(p);if(it==states.end())return -1;const auto& s=it->second.heat;return field==0?s.phase:field==1?s.heat:s.timer;}
bool KeepsNativeBlink(const Plant* p){return IsCustom(p);}
int EffectiveBase(const Plant* p){return int(p->mSeedType)==48?int(p->mImitaterType):int(p->mSeedType);}
int Power(const Plant* p){return SandboxMemeRules::PowerOf(Type(p));}
void Assign(Plant* p,int id){
 if(MemeCharacters::Is(id)){MemeCharacters::Assign(p,id);return;}
 if(!Find(id)||Base(id)!=EffectiveBase(p))return;auto& s=states[p];s={};s.id=id;s.health=p->mPlantHealth;s.heat=SandboxMemeRules::Initial(id);
 if(SandboxMemeRules::LegacyBase(Base(id))&&int(p->mSeedType)!=48){p->mLaunchCounter=9999;p->mShootingCounter=0;}
 else if(Power(p)==182&&p->mLaunchRate>0)p->mLaunchCounter=std::max(p->mLaunchCounter,p->mLaunchRate*2);
}
PowerSave SavePower(const Plant* p){
 if(MemeCharacters::Is(p))return MemeCharacters::Save(p);
 auto it=states.find(p);if(it==states.end())return {};
 const auto& s=it->second;const auto& h=s.heat;
 return {s.id,s.health,h.phase,h.heat,h.timer,h.delay,h.age,h.pulse,s.remaining,s.spacing};
}
bool RestorePower(Plant* p,const PowerSave& a){
 if(MemeCharacters::Is(a[0]))return MemeCharacters::Restore(p,a);
 if(!Find(a[0])||Base(a[0])!=EffectiveBase(p)||p->mDead||a[1]<0||a[1]>p->mPlantMaxHealth||a[2]<0||a[2]>2||a[3]<0||a[3]>1000||a[4]<0||a[4]>10000||a[5]<0||a[5]>10000||a[6]<0||a[6]>=1000000||a[7]<0||a[7]>32||a[8]<0||a[8]>12||a[9]<0||a[9]>8)return false;
 auto& s=states[p];s={};s.id=a[0];s.health=a[1];s.empowered=0;
 s.heat={a[2],a[3],a[4],a[5],a[6],a[7]};s.remaining=a[8];s.spacing=a[9];
 if(SandboxMemeRules::LegacyBase(Base(a[0]))&&int(p->mSeedType)!=48){p->mLaunchCounter=9999;p->mShootingCounter=0;}return true;
}
bool NativeCanAct(const Plant* p){
 auto it=states.find(p);if(it==states.end()||SandboxMemeRules::LegacyBase(EffectiveBase(p)))return true;
 const auto& h=it->second.heat;return !(Power(p)==180&&h.phase==SandboxMemeRules::Recovering)&&!(Power(p)==182&&h.delay>0);
}
int NativeCooldown(const Plant* p,int ticks){
 auto it=states.find(p);if(it==states.end()||SandboxMemeRules::LegacyBase(EffectiveBase(p)))return ticks;
 const auto& h=it->second.heat;const int power=Power(p),base=EffectiveBase(p);
 if(power==181)return std::max(20,ticks*(1000-h.heat*6/10)/1000);
 if(power==182)return std::min(10000,ticks*(base==6||base==47?3:4)/2);
 return std::max(20,ticks*((base==6||base==31||base==45||base==47)?65:h.phase==SandboxMemeRules::Bursting?55:100)/100);
}
void NativeAction(Plant* p){
 auto it=states.find(p);if(it==states.end()||SandboxMemeRules::LegacyBase(EffectiveBase(p)))return;
 using namespace SandboxMemeRules;auto& h=it->second.heat;const int power=Power(p),base=EffectiveBase(p);h.pulse=20;
 if(power==180&&h.phase==Warming){h.heat=std::min(1000,h.heat+(KindOf(base)==Kind::Producer?334:200));if(h.heat==1000){h.phase=Bursting;h.timer=180;}}
 if(power==181)h.heat=std::min(1000,h.heat+100);
 if(power==182){h.delay=std::max(300,NativeCooldown(p,GetPlantDefinition(p->mSeedType).mLaunchRate));h.heat=0;
  if(base==6){p->mPlantHealth=std::min(p->mPlantMaxHealth,p->mPlantHealth+120);it->second.health=p->mPlantHealth;}}
 if(p->mLaunchRate>0)p->mLaunchCounter=NativeCooldown(p,GetPlantDefinition(p->mSeedType).mLaunchRate);
}
int NativeProduction(Plant* p){
 if(!IsCustom(p)||SandboxMemeRules::LegacyBase(EffectiveBase(p)))return 1;
 NativeAction(p);auto& h=states.at(p).heat;
 if(Power(p)==182)return 4;
 if(Power(p)==180&&h.phase==SandboxMemeRules::Bursting){SandboxMemeRules::Recover(h,900);return 4;}return 1;
}
int NativeDamage(const Plant* p,int damage){
 auto it=states.find(p);if(it==states.end()||SandboxMemeRules::LegacyBase(EffectiveBase(p)))return damage;
 const int base=EffectiveBase(p),power=Power(p);const auto& h=it->second.heat;
 const int percent=power==182?300:power==180&&h.phase==SandboxMemeRules::Bursting?150:power==181&&(base==21||base==46)?100+h.heat/20:100;
 return damage*percent/100;
}
int ShotDamage(const Projectile* p,int damage){auto it=damageShots.find(p);return damage*(it==damageShots.end()?100:it->second)/100;}
// Existing optional shot record: low 16 bits damage %, upper bits trajectory.
// Old 100..300 records remain valid; native age/velocity are saved by the engine.
int SaveShot(const Projectile* p){return ShotDamage(p,100)|(MemeCharacters::ShotStyle(p)<<16);}
void RestoreShot(const Projectile* p,int record){
 if(record<0)return;const int percent=record&65535,style=record>>16;
 if(percent>=100&&percent<=300&&MemeCharacters::RestoreShotStyle(p,style))damageShots[p]=percent;
}
int ShotBlastRadius(const Projectile* p,int radius){return ShotDamage(p,100)>=300?radius*14/10:radius;}
void TorchPower(Plant* p,Projectile* shot){
 if(!IsCustom(p))return;auto& h=states.at(p).heat;
 if(Power(p)==182&&h.delay>0)return;
 NativeAction(p);const int percent=Power(p)==181?100+h.heat/20:Power(p)==182?250:125;
 damageShots[shot]=std::max(ShotDamage(shot,100),percent);h.pulse=20;
 if(Power(p)==182)h.delay=400;
}
void OneShot(Plant* p,Zombie* exclude){
 auto it=states.find(p);if(it==states.end()||SandboxMemeRules::KindOf(EffectiveBase(p))!=SandboxMemeRules::Kind::Instant||it->second.remaining)return;
 it->second.remaining=1;it->second.heat.pulse=32;const int power=Power(p);auto* b=p->mBoard;
 if(power==181){
  if(b->mCoins.mSize<b->mCoins.mMaxSize-8)for(int i=0;i<4;++i)b->AddCoin(p->mX+i*12,p->mY,COIN_SUN,COIN_MOTION_FROM_PLANT);
  if(b->mSeedBank)for(int i=0;i<b->mSeedBank->mNumPackets;++i){auto& card=b->mSeedBank->mSeedPackets[i];
   if((int(card.mPacketType)==EffectiveBase(p)||int(card.mPacketType)==48&&int(card.mImitaterType)==EffectiveBase(p))&&card.mRefreshing)
    card.mRefreshCounter=std::min(card.mRefreshTime,card.mRefreshCounter+card.mRefreshTime/2);
  }
  gLawnApp->PlayFoley(FOLEY_SPAWN_SUN);return;
 }
 for(auto* z:b->mZombies)if(z!=exclude&&Enemy(z)&&std::abs(z->mRow-p->mRow)<=1&&std::abs(z->mPosX-(p->mX+40))<190&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))){
  if(power==182){z->ApplyChill(false);continue;}
  z->TakeDamage(80,0U);
  if(z->mZombieType!=ZOMBIE_GARGANTUAR&&z->mZombieType!=ZOMBIE_REDEYE_GARGANTUAR&&z->mZombieType!=ZOMBIE_ZAMBONI){z->mPosX=std::min(850.0f,z->mPosX+65);z->UpdateReanim();}
 }
}
void NativeTint(const Plant* p,Sexy::Color& color){
 if(MemeCharacters::Is(p)){MemeCharacters::Tint(p,color);return;}
 auto it=states.find(p);if(it==states.end()||SandboxMemeRules::LegacyBase(int(p->mSeedType)))return;
 const int heat=it->second.heat.heat,power=Power(p),amount=20+heat/20;
 if(power==180){color.mGreen=color.mGreen*(255-amount)/255;color.mBlue=color.mBlue*(245-amount)/255;}
 else if(power==181)color.mBlue=color.mBlue*(220-heat/25)/255;
 else {color.mRed=color.mRed*205/255;color.mGreen=color.mGreen*230/255;}
}
void AdjustScale(const Plant* p,float& x,float& y,float& sx,float& sy){
 if(MemeCharacters::Is(p)){MemeCharacters::Scale(p,x,y,sx,sy);return;}
 auto it=states.find(p);if(it==states.end()||p->mSquished||!SandboxMemeRules::LegacyBase(Base(it->second.id))||int(p->mSeedType)==48)return;const auto& h=it->second.heat;
 const float swell=h.phase==SandboxMemeRules::Recovering?std::sin(h.age*0.09f)*0.025f:h.heat/1000.0f*0.035f+std::sin(h.age*0.35f)*(h.phase==SandboxMemeRules::Bursting?0.012f:0.0f);
 x-=40*sx*swell;y+=65*sy*swell;sx*=1+swell;sy*=1-swell;
}
void AdjustShadow(const Plant*,float&,float&,float&){}
float ShotScale(const Projectile*){return 1;}
bool HasShot(const Projectile* p){return shots.contains(p)||MemeCharacters::ShotStyle(p)!=0;}
bool UsesCustomShotArt(const Projectile*){return false;}
int ShotRadius(const Projectile*){return 12;}
int NextShot(Plant*){return 0;}
void OnFired(Plant* p,Projectile* shot,Zombie*){
 if(MemeCharacters::Is(p)){MemeCharacters::OnFired(p,shot);return;}
 if(!IsCustom(p))return;
 if(!SandboxMemeRules::LegacyBase(EffectiveBase(p))){const int percent=NativeDamage(p,100);if(percent!=100)damageShots[shot]=percent;return;}
 shots.insert(shot);
 const int base=int(p->mSeedType);auto reanim=p->mHeadReanimID;const char* track="idle_mouth";float w=35,h=49;
 if(base==18){const int head=SandboxVisualRules::HeadForRow(p->mRow,shot->mRow);
  reanim=head==1?p->mHeadReanimID:head==2?p->mHeadReanimID2:p->mHeadReanimID3;
  track=head==1?"ThreePeater_mouth1":head==2?"ThreePeater_mouth2":"ThreePeater_mouth3";w=19;h=43;
 }else if(base==40){track="GatlingPea_barrel3";w=43;h=27;}
 // SnowPea's mouth has its own canvas; preserve native origin if no track exists.
 if(base==5){track="SnowPea_mouth";w=33;h=49;}
 float x,y;if(SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(reanim),track,w,h,w-3,h*0.5f,x,y)){
  shot->mPosX=p->mX+x-12;shot->mPosY=p->mY+y-12-shot->mPosZ;
  shot->mX=int(shot->mPosX);shot->mY=int(shot->mPosY+shot->mPosZ);
 }
}
void UpdateShot(Projectile* p){MemeCharacters::UpdateShot(p);}
bool Impact(Projectile*,Zombie*){return false;} // Native damage, slow, fire and splats.
void Tick(Board* b){
 if(b->mPaused)return;
 MemeCharacters::Tick(b);
 using namespace SandboxMemeRules;
 for(auto* p:b->mPlants){
  if(p->mDead){Forget(p);continue;}auto it=states.find(p);if(it==states.end())continue;auto& s=it->second;
  if(int(p->mSeedType)==48)continue;
  const int base=Base(s.id),role=Role(s.id);const auto kind=KindOf(base);const bool legacy=LegacyBase(base);
  if(legacy){p->mLaunchCounter=9999;p->mShootingCounter=0;}if(s.empowered>0)--s.empowered;
  const int hurt=std::max(0,s.health-p->mPlantHealth);s.health=p->mPlantHealth;
  if(p->mIsAsleep||p->mSquished||p->NotOnGround()||p->mPlantHealth<=0)continue;
  if(!legacy&&kind!=Kind::Defense&&kind!=Kind::Support&&kind!=Kind::Magnet){
   auto& h=s.heat;h.age=(h.age+1)%1000000;if(h.pulse>0)--h.pulse;if(h.delay>0)--h.delay;
   if(h.phase==Bursting&&--h.timer<=0)Recover(h,240);
   else if(h.phase==Recovering){if(h.timer>0)--h.timer;h.heat=std::min(1000,h.timer*1000/(kind==Kind::Producer?900:240));if(!h.timer){h.phase=Warming;h.heat=0;}}
   if(Power(p)==182)h.heat=std::clamp(1000-h.delay*1000/std::max(400,NativeCooldown(p,GetPlantDefinition(p->mSeedType).mLaunchRate)),0,1000);
   if((kind==Kind::Shooter||kind==Kind::Producer)&&GetPlantDefinition(p->mSeedType).mLaunchRate>0)p->mLaunchRate=NativeCooldown(p,GetPlantDefinition(p->mSeedType).mLaunchRate);
   if(kind==Kind::Producer&&h.phase==Recovering)++p->mLaunchCounter;
   continue;
  }
  const bool aura=kind==Kind::Support||kind==Kind::Magnet;
  int watchedDamage=hurt;
  if(aura){for(auto* ally:b->mPlants)if(!ally->mDead&&std::abs(ally->mRow-p->mRow)<=1&&std::abs(ally->mPlantCol-p->mPlantCol)<=1&&ally->mRecentlyEatenCountdown>0)++watchedDamage;
   if(Power(p)==181&&kind==Kind::Support)s.heat.heat=std::min(1000,s.heat.heat+1);}
  Zombie* target=nullptr;
  for(int row=std::max(0,p->mRow-(base==18));row<=std::min((b->StageHasPool()?6:5)-1,p->mRow+(base==18));++row)
   if(auto* z=p->FindTargetZombie(row,WEAPON_PRIMARY)){target=z;break;}
  const bool room=role==Pea?b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8&&s.remaining==0:role!=Sunflower||b->mCoins.mSize<b->mCoins.mMaxSize-8;
  const int event=Step(s.heat,s.id,target!=nullptr,watchedDamage,room);
  if(event==Shoot||event==ChargedShoot){s.remaining=Volley(base)*(event==ChargedShoot?3:1);s.spacing=0;}
  if(s.spacing>0)--s.spacing;
  if(s.remaining>0&&!s.spacing&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8){
   for(int row=std::max(0,p->mRow-(base==18));row<=std::min((b->StageHasPool()?6:5)-1,p->mRow+(base==18));++row)p->Fire(target,row,WEAPON_PRIMARY);
   --s.remaining;s.spacing=8;gLawnApp->PlayFoley(FOLEY_THROW);
   int headIndex=0;
   for(auto id:{p->mHeadReanimID,p->mHeadReanimID2,p->mHeadReanimID3}){
    const char* layer=base==18?(headIndex==0?"anim_shooting1":headIndex==1?"anim_shooting2":"anim_shooting3"):"anim_shooting";
    ++headIndex;if(auto* head=gLawnApp->ReanimationTryToGet(id);head&&head->TrackExists(layer))head->PlayReanim(layer,REANIM_PLAY_ONCE_AND_HOLD,3,s.heat.phase==Bursting?70.0f:35.0f);
   }
  }
  if(event==Sun||event==ManySuns){for(int i=0;i<(event==ManySuns?4:1);++i)b->AddCoin(p->mX+i*8,p->mY,COIN_SUN,COIN_MOTION_FROM_PLANT);gLawnApp->PlayFoley(FOLEY_SPAWN_SUN);}
  if(event==Heal){const int amount=PowerOf(s.id)==181?40+s.heat.heat/10:60;
   if(aura){for(auto* ally:b->mPlants)if(!ally->mDead&&ally->mPlantHealth>0&&!ally->mSquished&&std::abs(ally->mRow-p->mRow)<=1&&std::abs(ally->mPlantCol-p->mPlantCol)<=1)ally->mPlantHealth=std::min(ally->mPlantMaxHealth,ally->mPlantHealth+amount);}
   else p->mPlantHealth=std::min(p->mPlantMaxHealth,p->mPlantHealth+amount);s.health=p->mPlantHealth;}
  if(event==Push)for(auto* z:b->mZombies)if(Enemy(z)&&std::abs(z->mRow-p->mRow)<=(aura?1:0)&&z->mPosX>=p->mX-25&&z->mPosX<p->mX+150&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))){
   if(z->mZombieType!=ZOMBIE_GARGANTUAR&&z->mZombieType!=ZOMBIE_REDEYE_GARGANTUAR&&z->mZombieType!=ZOMBIE_ZAMBONI){z->mPosX=std::min(850.0f,z->mPosX+65);z->UpdateReanim();}
  }
 }
}
void DrawEffects(Sexy::Graphics* graphics,Board* b,int row){
 Sexy::Graphics clipped(*graphics);clipped.ClipRect(0,82,800,518);auto* g=&clipped;
 MemeCharacters::Effects(g,b,row);
 for(const auto& [p,s]:states)if(!p->mDead&&!p->mIsAsleep&&!p->mSquished&&p->mRow==row){
  const auto& h=s.heat;
  if(s.empowered>0){const int age=32-s.empowered;for(int side:{-1,1})NativePuff(g,p->mX+40+side*(18+age*0.6f),p->mY+48-age*0.8f,12+age*0.4f,age,s.empowered*4);}
  if(SandboxMemeRules::PowerOf(s.id)!=180)continue;
  if(h.heat<600&&h.phase!=SandboxMemeRules::Recovering)continue;
  for(int i=0;i<2;++i){const int age=(h.age+i*22)%45;if(age>=30)continue;
   NativePuff(g,p->mX+29+i*19,p->mY+12-age*0.65f+PlantDrawHeightOffset(b,const_cast<Plant*>(p),p->mSeedType,p->mPlantCol,p->mRow),10+age*0.3f,age,(30-age)*4*h.heat/1000);
  }
  if(SandboxMemeRules::Role(s.id)==SandboxMemeRules::Wallnut&&h.pulse>0)NativePuff(g,p->mX+75+(32-h.pulse)*1.5f,p->mY+48,32,h.pulse,120);
 }
}
bool DrawShot(Sexy::Graphics*,const Projectile*){return false;}
void DrawCard(Sexy::Graphics* g,int x,int y,int id){
 if(MemeCharacters::Is(id)){MemeCharacters::Card(g,x,y,id);return;}
 if(SandboxMemeRules::IsPower(id)){
  DrawSeedPacket(g,x,y,static_cast<SeedType>(id==180?20:id==181?35:9),SEED_NONE,0,255,false,false);
  Sexy::Graphics label(*g);label.SetClipRect(x+3,y+53,44,15);
  PvzpDrawImageCelScaledF(&label,Sexy::IMAGE_SEEDS,x,y,2,0,1,1);
  PvzpDrawString(g,id==180?"红温":id==181?"内卷":"摆烂",x+25,y+65,Sexy::FONT_BRIANNETOD12,Sexy::Color(140,38,24),DS_ALIGN_CENTER);return;
 }
 const auto* d=Find(id);if(!d){if(id>=0&&id<48)DrawSeedPacket(g,x,y,static_cast<SeedType>(id),SEED_NONE,0,255,false,false);return;}
 if(!SandboxMemeRules::LegacyBase(d->base)){
  DrawSeedPacket(g,x,y,static_cast<SeedType>(d->base),SEED_NONE,0,255,false,false);
  const int power=SandboxMemeRules::PowerOf(id);
  Sexy::Graphics label(*g);label.SetClipRect(x+3,y+53,44,15);
  PvzpDrawImageCelScaledF(&label,Sexy::IMAGE_SEEDS,x,y,2,0,1,1);
  PvzpDrawString(g,power==180?"红温":power==181?"内卷":"摆烂",x+25,y+65,Sexy::FONT_BRIANNETOD12,Sexy::Color(140,38,24),DS_ALIGN_CENTER);return;
 }
 PvzpDrawImageCelScaledF(g,Sexy::IMAGE_SEEDS,x,y,2,0,1,1);
 Sexy::Graphics clip(*g);clip.SetClipRect(x+3,y+9,44,48);SandboxArt::DrawFit(&clip,Card(id),x+4,y+10,42,44);
 PvzpDrawString(g,"0",x+25,y+65,Sexy::FONT_BRIANNETOD12,Sexy::Color(55,64,23),DS_ALIGN_CENTER);
}
bool DrawBody(Sexy::Graphics* g,const Plant* p,float,float,bool squished){
 if(MemeCharacters::Type(p)==500&&MemeCharacters::Data(p,0)==1&&!squished){
  auto* body=gLawnApp->ReanimationTryToGet(p->mBodyReanimID);if(!body)return false;
  WarmSkin warm;for(auto id:{p->mBodyReanimID,p->mHeadReanimID})warm.Apply(gLawnApp->ReanimationTryToGet(id),120,24,p->mPlantHealth,p->mPlantMaxHealth);
  warm.Apply(gLawnApp->ReanimationTryToGet(p->mBlinkReanimID),120,24,p->mPlantHealth,p->mPlantMaxHealth,true);
  body->Draw(g);HeatBrow(g,gLawnApp->ReanimationTryToGet(p->mHeadReanimID),24,230);return true;
 }
 auto it=states.find(p);if(it==states.end()||squished||!SandboxMemeRules::LegacyBase(int(p->mSeedType)))return false;const auto& s=it->second;
 WarmSkin warm;const int level=SandboxMemeRules::PowerOf(s.id)==180?2+s.heat.heat*22/1000:12+s.heat.heat*12/1000;
 auto* body=gLawnApp->ReanimationTryToGet(p->mBodyReanimID);if(!body)return false;
 for(auto id:{p->mBodyReanimID,p->mHeadReanimID,p->mHeadReanimID2,p->mHeadReanimID3})warm.Apply(gLawnApp->ReanimationTryToGet(id),s.id,level,p->mPlantHealth,p->mPlantMaxHealth);
 warm.Apply(gLawnApp->ReanimationTryToGet(p->mBlinkReanimID),s.id,level,p->mPlantHealth,p->mPlantMaxHealth,true);body->Draw(g);
 if((Base(s.id)==0||Base(s.id)==7)&&SandboxMemeRules::PowerOf(s.id)==180&&s.heat.phase!=SandboxMemeRules::Recovering)HeatBrow(g,gLawnApp->ReanimationTryToGet(p->mHeadReanimID),level,std::clamp((s.heat.heat-500)/2,0,255));
 return true;
}
}
