#include "CoinPlants.h"
#include "MemeCharacters.h"
#include "MemeAdventure.h"
#include "Sandbox.h"
#include "SandboxFactions.h"
#include "SandboxArt.h"
#include "SandboxZombies.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Plant.h"
#include "Lawn/Board.h"
#include "Lawn/Zombie.h"
#include "Lawn/Projectile.h"
#include "Lawn/System/PlayerInfo.h"
#include "Lawn/System/ReanimationLawn.h"
#include "PvzpLib/Reanimator.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"
#include <map>
#include <memory>
#include <algorithm>
#include <cmath>
namespace CoinPlants {
using namespace CoinPlantRules;
namespace {
std::map<const Plant*,State> states;
Save pending;
int fired[3]{},hits[3]{};
float Height(const Plant* p){return PlantDrawHeightOffset(p->mBoard,const_cast<Plant*>(p),p->mSeedType,p->mPlantCol,p->mRow);}
float RowHeight(const Plant* p){return p->mBoard->StageHasPool()||p->mBoard->StageHasRoof()?85.f:100.f;}
bool Spend(Plant* p,int kind){
 if(!Kind(kind))return false;
 if(gSandboxEnabled)return true; // Sandbox ammunition is free, even with a zero balance.
 auto* wallet=p->mApp->mPlayerInfo;
 const int cost=Units(kind,MemeCharacters::Type(p)==Flower);
 if(!wallet||wallet->mCoins<cost)return false;
 wallet->AddCoins(-cost);p->mBoard->ShowCoinBank();return true;
}
bool Active(const Plant* p){return !p->mDead&&!p->mSquished&&!p->mIsAsleep&&p->mPlantHealth>0&&!const_cast<Plant*>(p)->NotOnGround()&&p->mOnBungeeState==NOT_ON_BUNGEE;}
Sexy::Image* CoinImage(int kind){return kind==Silver?Sexy::IMAGE_REANIM_COIN_SILVER_DOLLAR:kind==Gold?Sexy::IMAGE_REANIM_COIN_GOLD_DOLLAR:Sexy::IMAGE_REANIM_DIAMOND;}
void DrawCoin(Sexy::Graphics* g,int kind,float x,float y,int age){
 auto* im=CoinImage(kind);if(!im)return;
 const float size=kind==Diamond?24.f:20.f;
 Sexy::SexyTransform2D m;m.LoadIdentity();m.m00=size/im->mWidth*(kind==Diamond?1.f:.72f+.28f*std::cos(age*.06f));m.m11=size/im->mHeight;
 m.m02=x+g->mTransX;m.m12=y+g->mTransY;
 const auto tint=g->GetColorizeImages()?g->GetColor():Sexy::Color::White;
 PvzpBltMatrix(g,im,m,g->mClipRect,tint,g->mDrawMode,Sexy::Rect(0,0,im->mWidth,im->mHeight));
}
Sexy::Image* Accessory(bool cigarette){
 static std::unique_ptr<Sexy::MemoryImage> images[2];auto& im=images[cigarette?1:0];if(im)return im.get();
 const int w=cigarette?24:34,h=cigarette?6:12;im=std::make_unique<Sexy::MemoryImage>();im->Create(w,h);auto* px=im->GetBits();std::fill(px,px+w*h,0u);
 auto rect=[&](int x,int y,int width,int height,unsigned colour){for(int j=y;j<y+height;++j)for(int i=x;i<x+width;++i)px[j*w+i]=colour;};
 if(cigarette){rect(0,1,24,4,0xff382f25);rect(1,2,6,2,0xffbd8650);rect(7,2,14,2,0xffeee8d5);rect(21,2,2,2,0xffa84924);}
 else{rect(0,0,34,3,0xff151719);rect(3,3,12,6,0xff151719);rect(19,3,12,6,0xff151719);rect(5,9,8,3,0xff151719);rect(21,9,8,3,0xff151719);rect(14,3,6,2,0xff151719);rect(5,3,3,2,0xff666b6c);rect(8,5,2,2,0xff41474a);rect(21,3,3,2,0xff666b6c);}
 im->BitsChanged();return im.get();
}
void OnBone(Sexy::Graphics* g,Reanimation* anim,const char* track,float nativeW,float nativeH,float x,float y,bool cigarette,float angle){
 if(!anim||!anim->TrackExists(track))return;const int index=anim->FindTrackIndex(track);ReanimatorTransform pose;anim->GetCurrentTransform(index,&pose);if(pose.mFrame<0||pose.mAlpha<=0)return;
 Sexy::SexyTransform2D bone;anim->GetTrackMatrix(index,bone);Sexy::SexyTransform2D m=bone;
 x-=nativeW*.5f;y-=nativeH*.5f;m.m02+=bone.m00*x+bone.m01*y+g->mTransX;m.m12+=bone.m10*x+bone.m11*y+g->mTransY;
 const float c=std::cos(angle),s=std::sin(angle);m.m00=bone.m00*c+bone.m01*s;m.m10=bone.m10*c+bone.m11*s;m.m01=-bone.m00*s+bone.m01*c;m.m11=-bone.m10*s+bone.m11*c;
 auto* im=Accessory(cigarette);const auto tint=g->GetColorizeImages()?g->GetColor():Sexy::Color::White;
 SandboxFactions::DrawOverlay(m,g->mTransX,g->mTransY);
 PvzpBltMatrix(g,im,m,g->mClipRect,tint,g->mDrawMode,Sexy::Rect(0,0,im->mWidth,im->mHeight));
}
}
bool ShooterSlot(int seed){return seed==ShooterBase&&(gSandboxEnabled||MemeAdventure::RosterEnabled());}
bool FlowerSlot(int seed){return seed==FlowerBase&&(gSandboxEnabled||MemeAdventure::RosterEnabled());}
void Reset(){states.clear();pending.clear();std::fill(fired,fired+3,0);std::fill(hits,hits+3,0);}
void Forget(Plant* p){states.erase(p);}
void Update(Plant* p){
 const int id=MemeCharacters::Type(p);if(!IsPlant(id)||!p->mBoard||p->mBoard->mPaused||!p->IsInPlay()||!Active(p))return;
 auto [it,inserted]=states.try_emplace(p);auto& s=it->second;if(inserted){s.id=id;s.delay=Interval(id);}
 if(s.id!=id)return;
 s.phase=(s.phase+1)%OrbitPeriod;
 if(id==Flower){
  // Persistent money: one single-target hit per contact, rearmed only on separation.
  for(int slot=0;slot<Limit;++slot)if(s.coins[slot]){
   const auto pt=Orbit(slot,s.phase,RowHeight(p));const float x=p->mX+pt.x,y=p->mY+Height(p)+pt.y;
   unsigned contact=0;
   for(auto* z:p->mBoard->mZombies){
    if(z->mDead||!z->IsOnBoard()||z->IsDeadOrDying()||!SandboxFactions::Enemy(p,z)||SandboxZombies::IsHeld(z)||!z->EffectedByDamage(SandboxFactions::Flags(p,1)))continue;
    const auto r=z->GetZombieRect();
    if(std::abs(z->mRow-p->mRow)>1||r.mX+r.mWidth*.5f<p->mX-80||r.mX+r.mWidth*.5f>=p->mX+160)continue;
    if(x+9<r.mX||x-9>r.mX+r.mWidth||y+9<r.mY||y-9>r.mY+r.mHeight)continue;
    contact=p->mBoard->mZombies.DataArrayGetID(z);
    if(s.touching[slot]!=contact){const int kind=s.coins[slot];z->TakeDamage(Damage(kind),0);++hits[kind-1];p->mApp->PlayFoley(FOLEY_SPLAT);}
    break;
   }
   s.touching[slot]=contact;
   if(SandboxFactions::OrbitHit(p,slot,x,y,Damage(s.coins[slot]))){++hits[s.coins[slot]-1];p->mApp->PlayFoley(FOLEY_SPLAT);}
  }
 }
 if(s.delay>0)--s.delay;if(s.delay>0)return;
 auto* b=p->mBoard;Zombie* target=nullptr;
 if(id==Shooter){
  if(b->mProjectiles.mSize>=b->mProjectiles.mMaxSize-8)return;
  target=p->FindTargetZombie(p->mRow,WEAPON_PRIMARY);if(!target&&!SandboxFactions::Target(p,p->mRow))return;
 }else if(Count(s)>=Limit)return;
 // Roll once per production, never reroll every frame when a rare coin is unaffordable.
 if(!gSandboxEnabled&&(!p->mApp->mPlayerInfo||p->mApp->mPlayerInfo->mCoins<1))return;
 if(!s.pending)s.pending=Choose(Sexy::Rand(100),id==Flower);
 if(!Spend(p,s.pending))return;
 const int kind=s.pending;s.pending=0;s.delay=Interval(id);++fired[kind-1];
 if(id==Flower){
  *std::find(s.coins.begin(),s.coins.end(),0)=kind;
  p->mApp->PlayFoley(FOLEY_SPAWN_SUN);return;
 }
 float x=58,y=34;SandboxArt::TrackPoint(p->mApp->ReanimationTryToGet(p->mHeadReanimID),"idle_mouth",35,49,32,24.5f,x,y);
 // Spike collision/motion, money art: torchwood cannot turn paid coins into peas.
 auto* shot=b->AddProjectile(p->mX+x-12,p->mY+y-12,p->mRenderOrder-1,p->mRow,PROJECTILE_SPIKE);
 shot->mMotionType=MOTION_STRAIGHT;shot->mVelX=3.33f;shot->mVelY=0;shot->mDamageRangeFlags=1;
 MemeCharacters::RestoreShotStyle(shot,Style(kind));
 SandboxFactions::OnFired(p,shot,target);
 if(auto* head=p->mApp->ReanimationTryToGet(p->mHeadReanimID);head&&head->TrackExists("anim_shooting"))head->PlayReanim("anim_shooting",REANIM_PLAY_ONCE_AND_HOLD,3,35);
 p->mApp->PlayFoley(FOLEY_THROW);
}
bool Impact(Projectile* shot,Zombie* target){
 const int style=MemeCharacters::ShotStyle(shot);if(!Shot(style))return false;
 if(target&&!target->mDead&&!target->IsDeadOrDying()){const int kind=ShotKind(style);target->TakeDamage(Damage(kind),shot->GetDamageFlags(target));++hits[kind-1];shot->mApp->PlayFoley(FOLEY_SPLAT);}
 shot->Die();return true;
}
Save Capture(Board* b){
 Save out;for(auto* p:b->mPlants)if(!p->mDead){auto it=states.find(p);if(it!=states.end())out.push_back({b->mPlants.DataArrayGetID(p),it->second});}return out;
}
void Load(const Save& saved){pending=saved;}
void Restore(Board* b){
 for(const auto& record:pending)if(Valid(record.state))if(auto* p=b->mPlants.DataArrayTryToGet(record.key);p&&!p->mDead&&MemeCharacters::Type(p)==record.state.id)states[p]=record.state;
 pending.clear(); // Never debit again or restore the player's wallet from a board save.
}
bool DrawShot(Sexy::Graphics* g,const Projectile* shot){
 const int style=MemeCharacters::ShotStyle(shot);if(!Shot(style))return false;
 DrawCoin(g,ShotKind(style),shot->mPosX-shot->mX+12,shot->mPosY+shot->mPosZ-shot->mY+12,shot->mProjectileAge);return true;
}
void DrawGear(Sexy::Graphics* g,Reanimation* head,bool flower){
 OnBone(g,head,"anim_face",flower?44:70,flower?39:65,flower?23:49,flower?16:21,false,-.18f);
 if(!flower)OnBone(g,head,"idle_mouth",35,49,36,32,true,.12f);
}
void DrawPlant(Sexy::Graphics* g,const Plant* p){
 const int id=MemeCharacters::Type(p);if(!IsPlant(id)||p->mSquished)return;
 DrawGear(g,p->mApp->ReanimationTryToGet(id==Flower?p->mBodyReanimID:p->mHeadReanimID),id==Flower);
}
void DrawOrbit(Sexy::Graphics* g,const Plant* p,bool front){
 const auto it=states.find(p);if(it==states.end()||it->second.id!=Flower||!Active(p))return;const auto& s=it->second;
 for(int i=0;i<Limit;++i)if(s.coins[i]){const auto pt=Orbit(i,s.phase,RowHeight(p));if((pt.y>=40)==front)DrawCoin(g,s.coins[i],pt.x,Height(p)+pt.y,s.phase+i*9);}
}
void DrawPreview(Sexy::Graphics* g,float x,float y,bool flower,bool imitater){
 static std::unique_ptr<Sexy::MemoryImage> images[4];auto& im=images[(flower?2:0)+(imitater?1:0)];
 if(!im){im=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(120,120);Sexy::Graphics canvas(im.get());canvas.SetLinearBlend(true);
  Reanimation preview;preview.ReanimationInitializeType(20,20,flower?REANIM_MARIGOLD:REANIM_PEASHOOTER);preview.SetFramesForLayer(flower?"anim_idle":"anim_full_idle");
  if(imitater)gLawnApp->mReanimatorCache->UpdateReanimationForVariation(&preview,VARIATION_IMITATER);
  preview.Draw(&canvas);DrawGear(&canvas,&preview,flower);
 }
 PvzpDrawImageScaledF(g,im.get(),x-20*g->mScaleX,y-20*g->mScaleY,g->mScaleX,g->mScaleY);
}
}
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
// Read-only QA. No wallet setter, forced odds or adventure cheats.
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_coinplant_data(int kind,int index,int field){
 auto* b=gLawnApp?gLawnApp->mBoard:nullptr;if(!b)return -1;
 if(kind==0)return gLawnApp->mPlayerInfo?gLawnApp->mPlayerInfo->mCoins*10:-1;
 if(kind==1&&index>=0&&index<3)return field==0?CoinPlants::fired[index]:CoinPlants::hits[index];
 const auto save=CoinPlants::Capture(b);if(index<0||index>=int(save.size()))return -1;const auto& s=save[index].state;
 if(kind==3)return field>=0&&field<CoinPlantRules::Limit?s.coins[field]:-1;
 return field==0?s.id:field==1?s.delay:field==2?s.phase:field==3?s.pending:field==4?CoinPlantRules::Count(s):int(save[index].key);
}
#endif
