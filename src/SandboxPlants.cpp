// Independent characters; all other plants retain native behavior.
#include "SandboxPlants.h"
#include "SandboxArt.h"
#include "SandboxMemeRules.h"
#include "SandboxVisualRules.h"
#include "MemeShooterRules.h"
#include "AbstractRigVisuals.h"
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
std::map<const Projectile*,int> damageShots;
// Temporary head/mouth/blink tint; never mutate the shared native reanimation.
struct WarmSkin {
 std::vector<std::pair<ReanimatorTrackInstance*,Sexy::Image*>> previous;
 void Apply(Reanimation* anim,int level,bool blink=false,bool gatling=false){
  if(!anim)return;
  for(int i=0;i<anim->mDefinition->mTracks.count;++i){
   const std::string_view name=anim->mDefinition->mTracks.tracks[i].mName;const char* file=nullptr;
   if(!blink&&name.starts_with("anim_face"))file=gatling?"GatlingPea_head.png":"PeaShooter_Head.png";
   if(!blink&&!gatling&&name=="idle_mouth")file="PeaShooter_mouth.png";
   if(!blink&&gatling&&name=="GatlingPea_mouth")file="GatlingPea_mouth.png";
   if(!blink&&gatling&&name=="GatlingPea_mouth_overlay")file="GatlingPea_mouth_overlay.png";
   if(name=="idle_shoot_blink"||name.starts_with("anim_blink"))file=gatling?(anim->mAnimTime<.5f?"GatlingPea_blink1.png":"GatlingPea_blink2.png"):(anim->mAnimTime<.5f?"PeaShooter_blink1.png":"PeaShooter_blink2.png");
   if(!file)continue;
   auto& track=anim->mTrackInstances[i];previous.emplace_back(&track,track.mImageOverride);
   track.mImageOverride=SandboxArt::WarmNative(file,level);
  }
 }
 ~WarmSkin(){for(auto it=previous.rbegin();it!=previous.rend();++it)it->first->mImageOverride=it->second;}
};
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
void NutBrows(Sexy::Graphics* g,Reanimation* body,int alpha){
 if(!body||!body->TrackExists("anim_face"))return;
 auto* brow=SandboxArt::NativeImage("PeaShooter_eyebrow.png");if(!brow)return;
 Sexy::SexyTransform2D face;body->GetTrackMatrix(body->FindTrackIndex("anim_face"),face);
 // Reuse one native eyebrow, mirrored over the walnut's two eyes. Register
 // on its face bone, not the board, so idle sway/recoil/damage frames align.
 for(int side:{-1,1}){
  const float x=(side<0?44.0f:73.0f)-50,y=(side<0?29.0f:26.0f)-50;
  const float sx=-side*1.7f,sy=1.3f,c=0.966f,s=-side*0.259f;
  Sexy::SexyTransform2D m=face;
  m.m02+=face.m00*x+face.m01*y+g->mTransX;m.m12+=face.m10*x+face.m11*y+g->mTransY;
  m.m00=(face.m00*c+face.m01*s)*sx;m.m10=(face.m10*c+face.m11*s)*sx;
  m.m01=(-face.m00*s+face.m01*c)*sy;m.m11=(-face.m10*s+face.m11*c)*sy;
  PvzpBltMatrix(g,brow,m,g->mClipRect,Sexy::Color(85,65,35,alpha),g->mDrawMode,Sexy::Rect(0,4,13,7));
 }
}

}
void Reset(){damageShots.clear();MemeCharacters::Reset();}
void Forget(Plant* p){MemeCharacters::Forget(p);}
void ForgetShot(Projectile* p){damageShots.erase(p);MemeCharacters::ForgetShot(p);}
bool IsCustom(const Plant* p){return MemeCharacters::Is(p);}
int Type(const Plant* p){return IsCustom(p)?MemeCharacters::Type(p):int(p->mSeedType);}
int GrowthStage(const Plant*){return 0;}
int HeatData(const Plant* p,int field){return MemeCharacters::Data(p,field);}
bool KeepsNativeBlink(const Plant* p){return IsCustom(p);}
int EffectiveBase(const Plant* p){return int(p->mSeedType)==48?int(p->mImitaterType):int(p->mSeedType);}
int Power(const Plant*){return 0;}
void Assign(Plant* p,int id){MemeCharacters::Assign(p,id);}
PowerSave SavePower(const Plant* p){return MemeCharacters::Save(p);}
bool RestorePower(Plant* p,const PowerSave& saved){return MemeCharacters::Restore(p,saved);}
void RestoreRetired(Plant* p,const PowerSave& saved){
 if(p->mDead||(!MemeCharacters::IsRetired(saved[0])&&!SandboxMemeRules::IsResult(saved[0])))return;
 Forget(p);
 if(int(p->mSeedType)==52){p->mSeedType=SEED_PEASHOOTER;p->mX=p->mBoard->GridToPixelX(p->mPlantCol,p->mRow);p->mY=p->mBoard->GridToPixelY(p->mPlantCol,p->mRow);}
 if(saved[0]==518){p->mX=p->mBoard->GridToPixelX(p->mPlantCol,p->mRow);p->mY=p->mBoard->GridToPixelY(p->mPlantCol,p->mRow);p->mState=STATE_NOTREADY;p->PlayBodyReanim("anim_idle",REANIM_LOOP,5,12);}
 if(saved[0]==502&&!p->mBoard->StageIsNight())p->SetSleeping(true);
 p->mLaunchRate=GetPlantDefinition(p->mSeedType).mLaunchRate;p->mLaunchCounter=std::max(100,p->mLaunchRate);p->mShootingCounter=0;
}
bool NativeCanAct(const Plant*){return true;}
void NativeAction(Plant*){}
int NativeProduction(Plant*){return 1;}
int NativeCooldown(const Plant*,int ticks){return ticks;}
int NativeDamage(const Plant*,int damage){return damage;}
void OneShot(Plant*,Zombie*){}
void TorchPower(Plant*,Projectile*){}
void NativeTint(const Plant* p,Sexy::Color& color){MemeCharacters::Tint(p,color);}
int ShotDamage(const Projectile* shot,int damage){
 const int style=MemeCharacters::ShotStyle(shot);
 if(MemeCharacters::IsPalmShot(style))return MemeCharacters::PalmDamage(style);
 if(style==MemeCharacters::WeakProjectile)return std::max(1,damage/20);
 if(style==MemeCharacters::GatlingProjectile)return damage*15/20;
 auto it=damageShots.find(shot);return it==damageShots.end()?damage:damage*it->second/100;
}
int ShotBlastRadius(const Projectile* shot,int radius){return ShotDamage(shot,100)>=300?radius*14/10:radius;}
int SaveShot(const Projectile* p){const int style=MemeCharacters::ShotStyle(p);return (MemeCharacters::IsStraightShot(style)?100:ShotDamage(p,100))|(style<<16);}
void RestoreShot(const Projectile* p,int record){
 if(record<0)return;const int percent=record&65535,style=record>>16;
 if(percent>=100&&percent<=300&&MemeCharacters::RestoreShotStyle(p,style)&&style!=0)damageShots[p]=percent;
}
void AdjustScale(const Plant* p,float& x,float& y,float& sx,float& sy){
 if(p->mSeedType==SEED_SMALL_NUT){
  x+=40*sx*(1-MemeCharacters::SmallNutScale);y+=75*sy*(1-MemeCharacters::SmallNutScale);
  sx*=MemeCharacters::SmallNutScale;sy*=MemeCharacters::SmallNutScale;
 }
 MemeCharacters::Scale(p,x,y,sx,sy);
}
void AdjustShadow(const Plant* p,float&,float&,float& scale){if(p->mSeedType==SEED_SMALL_NUT)scale*=MemeCharacters::SmallNutScale;}
float ShotScale(const Projectile* shot){
 // Visual only: weak Repeater peas are smaller, not weaker or slower.
 // Torchwood's attached fire animation keeps its native size and shadow.
 return MemeCharacters::ShotStyle(shot)==MemeCharacters::WeakProjectile&&
  (shot->mProjectileType==PROJECTILE_PEA||shot->mProjectileType==PROJECTILE_SNOWPEA)?0.6f:1.0f;
}
bool HasShot(const Projectile* p){return MemeCharacters::ShotStyle(p)!=0;}
bool UsesCustomShotArt(const Projectile* shot){return MemeCharacters::IsPalmShot(MemeCharacters::ShotStyle(shot));}
int ShotRadius(const Projectile*){return 12;}
int NextShot(Plant*){return 0;}
void OnFired(Plant* p,Projectile* shot,Zombie*){MemeCharacters::OnFired(p,shot);}
void UpdateShot(Projectile* p){MemeCharacters::UpdateShot(p);}
bool Impact(Projectile* shot,Zombie* zombie){
 const int style=MemeCharacters::ShotStyle(shot);if(!MemeCharacters::IsPalmShot(style))return false;
 if(!zombie||zombie->mDead||zombie->IsDeadOrDying())return true;
 zombie->TakeDamage(MemeCharacters::PalmDamage(style),shot->GetDamageFlags(zombie));
 if(!zombie->IsDeadOrDying()){
  const bool giant=zombie->mZombieType==ZOMBIE_GARGANTUAR||zombie->mZombieType==ZOMBIE_REDEYE_GARGANTUAR;
  zombie->mPosX+=MemeCharacters::PalmPush(style,giant);zombie->mX=int(zombie->mPosX);zombie->StopEating();
 }
 return true; // Native armor/shields took one hit, never double-apply damage.
}
void Tick(Board* b){MemeCharacters::Tick(b);}
void DrawEffects(Sexy::Graphics* graphics,Board* b,int row){
 Sexy::Graphics clipped(*graphics);clipped.ClipRect(0,82,800,518);MemeCharacters::Effects(&clipped,b,row);
}
bool DrawShot(Sexy::Graphics* g,const Projectile* shot){
 if(MemeCharacters::IsPalmShot(MemeCharacters::ShotStyle(shot))){
  auto* image=SandboxArt::Palm();if(!image)return false;
  Sexy::SexyTransform2D m;m.LoadIdentity();m.m00=40.f/image->mWidth;m.m11=27.5f/image->mHeight;
  m.m02=shot->mPosX-shot->mX+12+g->mTransX;m.m12=shot->mPosY+shot->mPosZ-shot->mY+12+g->mTransY;
  PvzpBltMatrix(g,image,m,g->mClipRect,Sexy::Color(255,255,255),g->mDrawMode,Sexy::Rect(0,0,image->mWidth,image->mHeight));return true;
 }
 if(MemeCharacters::ShotStyle(shot)!=MemeCharacters::ShooterProjectile)return false;
 // A whole original peashooter, including its stem/leaves/mouth. This cache
 // is intentionally the native seed 0, never the pea-headed preview.
 static std::unique_ptr<Sexy::MemoryImage> shooter;
 if(!shooter)shooter=gLawnApp->mReanimatorCache->MakeCachedPlantFrame(SEED_PEASHOOTER,VARIATION_NORMAL);
 if(!shooter)return false;
 const float angle=shot->mProjectileAge*.085f,c=std::cos(angle)*.56f,s=std::sin(angle)*.56f;
 Sexy::SexyTransform2D m;m.LoadIdentity();m.m00=c;m.m01=-s;m.m10=s;m.m11=c;
 // Projectile::Draw receives an object-local Graphics frame (already moved
 // to mX/mY). Only add the fractional position, never world position twice.
 m.m02=shot->mPosX-shot->mX+12+g->mTransX;m.m12=shot->mPosY+shot->mPosZ-shot->mY+12+g->mTransY;
 // Original 120px cache: plant's visual centre is five pixels below centre.
 m.m02+=5*s;m.m12-=5*c;
 PvzpBltMatrix(g,shooter.get(),m,g->mClipRect,Sexy::Color(255,255,255),g->mDrawMode,Sexy::Rect(0,0,shooter->mWidth,shooter->mHeight));
 return true;
}
void DrawPeaHeadPreview(Sexy::Graphics* g,float x,float y,bool imitater){
 // Separate cache: never alter vanilla backward-shooter or projectile art.
 static std::unique_ptr<Sexy::MemoryImage> previews[2];auto& cached=previews[imitater?1:0];
 if(!cached){
  cached=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(120,120);Sexy::Graphics canvas(cached.get());canvas.SetLinearBlend(true);
  for(const char* layer:{"anim_idle","anim_head_idle"}){
   Reanimation anim;anim.ReanimationInitializeType(20,20,REANIM_REPEATER);anim.SetFramesForLayer(layer);
   if(imitater)gLawnApp->mReanimatorCache->UpdateReanimationForVariation(&anim,VARIATION_IMITATER);
   AbstractRigVisuals::Scope pose(&anim,MemeCharacters::ShooterPea);anim.Draw(&canvas);
  }
 }
 PvzpDrawImageScaledF(g,cached.get(),x-20*g->mScaleX,y-20*g->mScaleY,g->mScaleX,g->mScaleY);
}
void DrawPalmPreview(Sexy::Graphics* g,float x,float y,bool imitater){
 static std::unique_ptr<Sexy::MemoryImage> previews[2];auto& cached=previews[imitater?1:0];
 if(!cached){
  cached=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(120,120);Sexy::Graphics canvas(cached.get());canvas.SetLinearBlend(true);
  Reanimation anim;anim.ReanimationInitializeType(20,20,REANIM_CACTUS);anim.SetFramesForLayer("anim_idle");
  if(imitater)gLawnApp->mReanimatorCache->UpdateReanimationForVariation(&anim,VARIATION_IMITATER);
  AbstractRigVisuals::Scope pose(&anim,MemeCharacters::CactusPalm);anim.Draw(&canvas);
 }
 PvzpDrawImageScaledF(g,cached.get(),x-20*g->mScaleX,y-20*g->mScaleY,g->mScaleX,g->mScaleY);
}
void DrawCard(Sexy::Graphics* g,int x,int y,int id){
 if(MemeCharacters::Is(id)){MemeCharacters::Card(g,x,y,id);return;}
 if(id>=0&&id<48)DrawSeedPacket(g,x,y,static_cast<SeedType>(id),SEED_NONE,0,255,false,false);
}
void DrawTuckingPreview(Sexy::Graphics* g,float x,float y,bool imitater){
 static std::unique_ptr<Sexy::MemoryImage> previews[2];auto& cached=previews[imitater?1:0];
 if(!cached){
  cached=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(120,120);Sexy::Graphics canvas(cached.get());canvas.SetLinearBlend(true);
  Reanimation anim;anim.ReanimationInitializeType(20,20,REANIM_SUNFLOWER);anim.SetFramesForLayer("anim_idle");
  if(imitater)gLawnApp->mReanimatorCache->UpdateReanimationForVariation(&anim,VARIATION_IMITATER);
  AbstractRigVisuals::Scope pose(&anim,MemeCharacters::TuckingSunflower);anim.Draw(&canvas);
 }
 PvzpDrawImageScaledF(g,cached.get(),x-20*g->mScaleX,y-20*g->mScaleY,g->mScaleX,g->mScaleY);
}
bool DrawBody(Sexy::Graphics* g,const Plant* p,float,float,bool squished){
 AbstractRigVisuals::Scope allPoses(p);
 if(MemeCharacters::Type(p)==501&&MemeCharacters::Data(p,0)==1&&!squished){
  auto* body=gLawnApp->ReanimationTryToGet(p->mBodyReanimID);if(!body)return false;
  body->Draw(g);NutBrows(g,body,std::min(255,MemeCharacters::Save(p)[7]*32));return true;
 }
 const bool gatling=MemeCharacters::Type(p)==MemeCharacters::GatlingShooter;
 if((MemeCharacters::Type(p)==500||gatling)&&!squished){
  auto* body=gLawnApp->ReanimationTryToGet(p->mBodyReanimID);if(!body)return false;
  const bool active=MemeCharacters::Data(p,0)==1;
  // Colour is a visual readout only: cooling gradually restores green without
  // modifying heat, attack clocks, native rig geometry or shared card art.
  const int level=std::clamp(gatling?(active?MemeCharacters::Data(p,2)*24/MemeCharacters::GatlingCooldown:MemeCharacters::Data(p,1)*24/MemeCharacters::GatlingHeatLimit):(active?24:MemeCharacters::Data(p,1)*24/MemeShooterRules::MaxRage),0,24);
  WarmSkin warm;for(auto id:{p->mBodyReanimID,p->mHeadReanimID})warm.Apply(gLawnApp->ReanimationTryToGet(id),level,false,gatling);
  warm.Apply(gLawnApp->ReanimationTryToGet(p->mBlinkReanimID),level,true,gatling);
  body->Draw(g);if(!gatling)HeatBrow(g,gLawnApp->ReanimationTryToGet(p->mHeadReanimID),level,std::clamp((level-8)*15,0,230));return true;
 }
 if(MemeCharacters::Is(p)&&!squished){if(auto* body=gLawnApp->ReanimationTryToGet(p->mBodyReanimID)){body->Draw(g);return true;}}
 return false;
}
}
