#include "EverythingShooter.h"
#include "EverythingShooterRules.h"
#include "MemeCharacters.h"
#include "MemeAdventure.h"
#include "Sandbox.h"
#include "SandboxArt.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Plant.h"
#include "Lawn/Board.h"
#include "Lawn/Zombie.h"
#include "Lawn/Projectile.h"
#include "Lawn/System/ReanimationLawn.h"
#include "PvzpLib/Reanimator.h"
#include "PvzpLib/PvzpParticle.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include "graphics/GLImage.h"
#include <algorithm>
#include <cmath>
#include <memory>
namespace EverythingShooter {
namespace {
int fired[17]{},impacts[17]{};
Sexy::Image* RareImage(int style){
 static std::unique_ptr<Sexy::MemoryImage> doom,cherry;
 static std::unique_ptr<Sexy::GLImage> poop;
 if(style==EverythingShooterRules::Poop){if(!poop)poop.reset(gLawnApp->GetImage("/addons/images/everything-poop.png"));return poop.get();}
 auto& cache=style==EverythingShooterRules::Doom?doom:cherry;
 if(!cache)cache=gLawnApp->mReanimatorCache->MakeCachedPlantFrame(style==EverythingShooterRules::Doom?SEED_DOOMSHROOM:SEED_CHERRYBOMB,VARIATION_NORMAL);
 return cache.get();
}
}
bool IsSlot(int seed){return seed==EverythingShooterRules::Base&&(gSandboxEnabled||MemeAdventure::RosterEnabled());}
bool Fire(Plant* p,Zombie* target){
 if(MemeCharacters::Type(p)!=EverythingShooterRules::Id)return false;
 auto* b=p->mBoard;if(!b||!target||target->mDead||b->mProjectiles.mSize>=b->mProjectiles.mMaxSize-8)return true;
 const int style=EverythingShooterRules::Choose(Sexy::Rand(100),Sexy::Rand(EverythingShooterRules::NativeCount));
 const int type=EverythingShooterRules::NativeType(style);
 // Use the real interpolated muzzle, including its attached moving head.
 float x=58,y=34;SandboxArt::TrackPoint(p->mApp->ReanimationTryToGet(p->mHeadReanimID),"idle_mouth",35,49,32,24.5f,x,y);
 const float ox=p->mX+x-12,oy=p->mY+y-12;
 auto* shot=b->AddProjectile(ox,oy,p->mRenderOrder-1,p->mRow,ProjectileType(type==PROJECTILE_FIREBALL?PROJECTILE_PEA:type));
 shot->mDamageRangeFlags=EverythingShooterRules::Lob(style)?13:1;
 if(EverythingShooterRules::Special(style))shot->mDamageRangeFlags=127;
 if(type==PROJECTILE_COBBIG){
  shot->mMotionType=MOTION_LOBBED;shot->mVelX=.001f;shot->mVelZ=-8;shot->mAccZ=0;
  shot->mCobTargetX=std::clamp(target->ZombieTargetLeadX(100)-80.f,0.f,720.f);shot->mCobTargetRow=p->mRow;shot->mDamageRangeFlags=127;
 }else if(EverythingShooterRules::Lob(style)){
  const auto rect=target->GetZombieRect();const float dx=std::max(40.f,target->ZombieTargetLeadX(50)-ox-30.f);
  shot->mMotionType=MOTION_LOBBED;shot->mVelX=dx/120;shot->mVelY=0;shot->mVelZ=(rect.mY-oy)/120-7;shot->mAccZ=.115f;
 }else{
  shot->mMotionType=type==PROJECTILE_PUFF?MOTION_PUFF:MOTION_STRAIGHT;shot->mVelX=3.33f;shot->mVelY=0;
 }
 if(type==PROJECTILE_FIREBALL)shot->ConvertToFireball(p->mPlantCol);
 MemeCharacters::RestoreShotStyle(shot,style);++fired[style-EverythingShooterRules::First];
 p->mApp->PlayFoley(type==PROJECTILE_PUFF?FOLEY_PUFF:FOLEY_THROW);return true;
}
bool Impact(Projectile* shot,Zombie* target){
 const int style=MemeCharacters::ShotStyle(shot);if(!EverythingShooterRules::Own(style))return false;
 ++impacts[style-EverythingShooterRules::First];
 if(!EverythingShooterRules::Special(style))return false; // Preserve native damage, butter, chill and splashes.
 auto* b=shot->mBoard;const float x=shot->mPosX+20,y=shot->mPosY+shot->mPosZ+20;
 if(style==EverythingShooterRules::Poop){
  if(target&&!target->mDead&&!target->IsDeadOrDying())target->TakeDamage(EverythingShooterRules::PoopDamage,shot->GetDamageFlags(target));
  if(auto* puff=shot->mApp->AddPvzpParticle(x,y,shot->mRenderOrder+1,PARTICLE_PUFF_SPLAT))puff->OverrideColor(nullptr,Sexy::Color(120,83,43));
  shot->mApp->PlayFoley(FOLEY_SPLAT);
 }else{
  const bool doom=style==EverythingShooterRules::Doom;
  b->KillAllZombiesInRadius(shot->mRow,x,y,doom?250:115,doom?3:1,true,127);
  shot->mApp->AddPvzpParticle(x,y,RENDER_LAYER_TOP,doom?PARTICLE_DOOM:PARTICLE_POWIE);
  if(doom)shot->mApp->PlaySample(Sexy::SOUND_DOOMSHROOM);else shot->mApp->PlayFoley(FOLEY_CHERRYBOMB);
  b->ShakeBoard(doom?3:2,doom?-4:-2);
 }
 shot->Die();return true;
}
bool DrawShot(Sexy::Graphics* g,const Projectile* shot){
 const int style=MemeCharacters::ShotStyle(shot);if(!EverythingShooterRules::Special(style))return false;
 auto* image=RareImage(style);if(!image)return false;
 const bool poop=style==EverythingShooterRules::Poop;
 const float width=poop?54:style==EverythingShooterRules::Doom?60:55,height=poop?48:width;
 const float angle=poop?-.15f:std::sin(shot->mProjectileAge*.06f)*.12f,c=std::cos(angle),s=std::sin(angle);
 Sexy::SexyTransform2D m;m.LoadIdentity();m.m00=c*width/image->mWidth;m.m10=s*width/image->mWidth;m.m01=-s*height/image->mHeight;m.m11=c*height/image->mHeight;
 m.m02=shot->mPosX-shot->mX+20+g->mTransX;m.m12=shot->mPosY+shot->mPosZ-shot->mY+20+g->mTransY;
 PvzpBltMatrix(g,image,m,g->mClipRect,Sexy::Color::White,g->mDrawMode,Sexy::Rect(0,0,image->mWidth,image->mHeight));return true;
}
void DrawGear(Sexy::Graphics* g,Reanimation* head){
 if(!head)return;float x=0,y=0;if(!SandboxArt::TrackPoint(head,"anim_face",70,65,9,18.5f,x,y))return;
 // Ammunition nests in rear leaves, following the head, never the muzzle.
 for(int i=0;i<2;++i){auto* image=SandboxArt::NativeImage(i?"Cornpult_kernal.png":"Cabbagepult_cabbage.png");if(!image)continue;
  Sexy::SexyTransform2D m;m.LoadIdentity();m.m00=(i?12.f:18.f)/image->mWidth;m.m11=17.f/image->mHeight;
  m.m02=x-8+i*10+g->mTransX;m.m12=y-i*2+g->mTransY;
  PvzpBltMatrix(g,image,m,g->mClipRect,Sexy::Color::White,g->mDrawMode,Sexy::Rect(0,0,image->mWidth,image->mHeight));
 }
}
void DrawPlant(Sexy::Graphics* g,const Plant* p){if(MemeCharacters::Type(p)==EverythingShooterRules::Id&&!p->mSquished)DrawGear(g,p->mApp->ReanimationTryToGet(p->mHeadReanimID));}
void DrawPreview(Sexy::Graphics* g,float x,float y,bool imitater){
 static std::unique_ptr<Sexy::MemoryImage> images[2];auto& image=images[imitater?1:0];
 if(!image){
  image=gLawnApp->mReanimatorCache->MakeBlankMemoryImage(120,120);
  Sexy::Graphics canvas(image.get());canvas.SetLinearBlend(true);
  // The original complete idle frame is a single coherent card silhouette.
  Reanimation preview;preview.ReanimationInitializeType(20,20,REANIM_PEASHOOTER);preview.SetFramesForLayer("anim_full_idle");
  if(imitater)gLawnApp->mReanimatorCache->UpdateReanimationForVariation(&preview,VARIATION_IMITATER);
  preview.Draw(&canvas);DrawGear(&canvas,&preview);
 }
 PvzpDrawImageScaledF(g,image.get(),x-20*g->mScaleX,y-20*g->mScaleY,g->mScaleX,g->mScaleY);
}
}
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
// Read-only counters, not forced rolls or adventure setters.
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_everything_data(int kind,int index){if(index<0||index>=17)return -1;return kind==0?EverythingShooter::fired[index]:kind==1?EverythingShooter::impacts[index]:-1;}
#endif
