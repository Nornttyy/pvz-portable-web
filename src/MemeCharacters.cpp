// Individual character mechanics using the native rigs and seed bank.
#include "MemeCharacters.h"
#include "MemeShooterRules.h"
#include "SandboxPlants.h"
#include "SandboxArt.h"
#include "SandboxZombies.h"
#include "AbstractRigVisuals.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "Lawn/Projectile.h"
#include "Lawn/SeedPacket.h"
#include "PvzpLib/Reanimator.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"
#include <map>
#include <algorithm>
#include <cmath>
#include <string>
namespace MemeCharacters {
namespace {
// Saved in the existing ten-integer optional plant record.
struct State {int id=0,health=0,phase=0,heat=0,timer=0,delay=50,age=0,pulse=0,remaining=0,direction=1;};
std::map<const Plant*,State> states;
std::map<const Projectile*,int> shotStyles; // 1..8 legacy wobble; 9 burst; 10 hit; 11..18 legacy miss; 20 legacy straight miss; 32..287 floating seeds.
void Burst(State& s){
 s.phase=1;s.heat=0;s.remaining=MemeShooterRules::BurstCount;s.delay=MemeShooterRules::BurstInterval(0);s.timer=0;s.pulse=30;
 gLawnApp->PlayRageRelease(); // Once per release, never once per pea or save restore.
}
bool Enemy(Zombie* z){return !z->mDead&&z->IsOnBoard()&&!z->mMindControlled&&!z->IsDeadOrDying()&&z->mHasHead&&!SandboxZombies::IsHeld(z);}
int VisualY(const Plant* p){return p->mY+int(std::lround(PlantDrawHeightOffset(p->mBoard,const_cast<Plant*>(p),p->mSeedType,p->mPlantCol,p->mRow)));}
void Shoot(Plant* p,Zombie* target){
 p->Fire(target,p->mRow,WEAPON_PRIMARY);
 if(auto* anim=gLawnApp->ReanimationTryToGet(p->mHeadReanimID);anim&&anim->TrackExists("anim_shooting"))anim->PlayReanim("anim_shooting",REANIM_PLAY_ONCE_AND_HOLD,3,35.0f);
}
void Puff(Sexy::Graphics* g,float x,float y,int age,int alpha){
 auto* im=SandboxArt::NativeImage(age<15?"puff_3.png":"puff_4.png");if(!im)return;
 Sexy::SexyTransform2D m;m.LoadIdentity();m.m00=(18+age*0.6f)/im->mWidth;m.m11=(18+age*0.6f)/im->mHeight;m.m02=x+g->mTransX;m.m12=y+g->mTransY;
 PvzpBltMatrix(g,im,m,g->mClipRect,Sexy::Color(255,255,255,alpha),g->mDrawMode,Sexy::Rect(0,0,im->mWidth,im->mHeight));
}
}
int Type(const Plant* p){auto it=states.find(p);return it==states.end()?0:it->second.id;}
bool Is(const Plant* p){return states.contains(p);}
bool Hiding(const Plant*){return false;}
bool Producing(const Plant*){return false;}
void Reset(){states.clear();shotStyles.clear();}
void Forget(Plant* p){states.erase(p);}
void Assign(Plant* p,int id){const auto* d=Find(id);if(!d||int(p->mSeedType)!=d->base)return;
 State s;s.id=id;s.health=p->mPlantHealth;s.timer=0;if(id==500)s.direction=2;states[p]=s;
 p->mLaunchCounter=9999;p->mShootingCounter=0;
}
std::array<int,10> Save(const Plant* p){auto it=states.find(p);if(it==states.end())return {};const auto& s=it->second;return {s.id,s.health,s.phase,s.heat,s.timer,s.delay,s.age,s.pulse,s.remaining,s.direction};}
bool Restore(Plant* p,const std::array<int,10>& a){
 const bool shooter=a[0]==500,newShooter=shooter&&a[9]==2;
 const auto* d=Find(a[0]);if(!d||int(p->mSeedType)!=d->base||p->mDead||a[1]<0||a[1]>p->mPlantMaxHealth||a[2]<0||a[2]>(newShooter?1:2)||a[3]<0||a[3]>(newShooter?300:1000)||a[4]<0||a[4]>2000||a[5]<0||a[5]>2000||a[6]<0||a[6]>=1000000||a[7]<0||a[7]>50||a[8]<0||a[8]>(newShooter?150:3)||(!newShooter&&a[9]!=1&&a[9]!=-1))return false;
 if(newShooter&&((a[2]==1&&(a[8]==0||a[3]!=0))||(a[2]==0&&a[8]!=0)))return false;
 State s{a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9]};
 // Direction was unused for this character: 2 versions the new burst state.
 // Migrate old overheating saves without healing or inventing a free volley.
 if(shooter&&!newShooter){s.phase=0;s.heat=a[2]==0?a[3]*300/1000:0;s.timer=0;s.delay=std::min(a[5],150);s.remaining=0;s.pulse=0;s.direction=2;}
 if(shooter)s.heat=std::min(s.heat,MemeShooterRules::MaxRage); // Keep old 300-rage saves readable.
 if(shooter)s.remaining=std::min(s.remaining,MemeShooterRules::BurstCount); // Read old 80/150-pea saves without adding shots.
 states[p]=s;p->mLaunchCounter=9999;p->mShootingCounter=0;return true;
}
int Data(const Plant* p,int field){const auto it=states.find(p);if(it==states.end())return -1;const auto& s=it->second;if(field==5)return -1;return field==0?s.phase:field==1?s.heat:field==2?s.timer:field==3?s.direction:s.remaining;}
// Keep legacy input/ABI callers harmless; rage is automatic only.
bool Activate(Plant*,int){return false;}
bool Click(Board*,int,int){return false;}
// Compatibility hooks now leave every other native plant unchanged.
bool RearmPotato(Plant*){return false;}
bool ReturnSquash(Plant*){return false;}
Zombie* PickTarget(Plant*,Zombie* nativeTarget){return nativeTarget;}
bool ButterReady(const Plant*){return false;}
bool StarTarget(Plant*){return false;}
void Tick(Board* b){
 if(b->mPaused)return;
 for(auto* p:b->mPlants){
  auto it=states.find(p);if(it==states.end())continue;if(p->mDead){states.erase(it);continue;}auto& s=it->second;
  p->mLaunchCounter=9999;p->mShootingCounter=0;
  if(p->mIsAsleep||p->mSquished||p->NotOnGround()||p->mPlantHealth<=0)continue;
  s.age=(s.age+1)%1000000;if(s.pulse)--s.pulse;if(s.delay)--s.delay;if(s.timer)--s.timer;
  if(s.id==500){
   if(s.phase==0&&s.heat>=MemeShooterRules::MaxRage)Burst(s);
   const bool room=b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8;
   if(s.phase==1){
    if(!s.delay&&room){
     Shoot(p,nullptr);--s.remaining;s.delay=MemeShooterRules::BurstInterval(MemeShooterRules::BurstCount-s.remaining);s.pulse=10;
     if(!s.remaining){s.phase=0;s.heat=0;s.delay=MemeShooterRules::RecoveryDelay;}
    }
   }else if(!s.delay&&room){
    if(auto* target=p->FindTargetZombie(p->mRow,WEAPON_PRIMARY)){
     Shoot(p,target);s.heat+=MemeShooterRules::PerShot;s.delay=MemeShooterRules::NormalDelay;
     if(s.heat>=MemeShooterRules::MaxRage)Burst(s);
    }
   }
  }else if(s.id==ShooterPea){
   if(!s.delay&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8){
    if(auto* target=p->FindTargetZombie(p->mRow,WEAPON_PRIMARY)){Shoot(p,target);s.delay=150;s.pulse=22;}
   }
  }else if(s.id==501){
   if(!s.timer&&p->mRecentlyEatenCountdown>0){
    Zombie* target=nullptr;
    for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mIsEating&&z->mPosX>=p->mX-65&&z->mPosX<p->mX+35&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))&&(!target||z->mPosX>target->mPosX))target=z;
    if(target){s.timer=300;s.pulse=50;s.phase=1;s.heat=int(target->mPosX-p->mX+65);gLawnApp->PlayFoley(FOLEY_THROW);}
   }
   if(s.phase==1&&s.pulse==28){
    for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->mPosX>=p->mX-65&&z->mPosX<p->mX+40&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))){
     z->TakeDamage(80,0);break;
    }
   }
   if(!s.pulse)s.phase=0;
  }
  s.health=p->mPlantHealth;
 }
}
void Tint(const Plant*,Sexy::Color&){}
void Scale(const Plant* p,float& x,float& y,float& sx,float& sy){auto it=states.find(p);if(it==states.end()||p->mSquished)return;const auto& s=it->second;
 float horizontal=1,vertical=1;
 if(s.id==500&&s.phase==1){const float recoil=std::sin(s.age*0.16f)*0.04f;horizontal+=0.10f+recoil;vertical-=0.07f+recoil;x-=sx*(4+6*(1-s.remaining/50.0f));}
 if(s.id==501&&s.phase==1&&s.pulse){
  // Keep the original face, damage frames and grid anchor during the bump.
  const float age=50-s.pulse;float offset=0;
  if(age<10){const float q=age/10;offset=-8*q;horizontal=1+0.30f*q;vertical=1-0.18f*q;}
  else if(age<22){const float q=(age-10)/12,e=q*q*(3-2*q);offset=-8+36*e;horizontal=1.30f-0.44f*e;vertical=0.82f+0.31f*e;}
  else{const float q=(age-22)/28,e=q*q*(3-2*q);offset=28*(1-e);horizontal=0.86f+0.14f*e;vertical=1.13f-0.13f*e;}
  x+=std::abs(sx)*offset;
 }
 x+=40*sx*(1-horizontal);y+=65*sy*(1-vertical);sx*=horizontal;sy*=vertical;
}
void Effects(Sexy::Graphics* g,Board* b,int row){
 for(const auto& [p,s]:states)if(!p->mDead&&p->mRow==row&&!p->mSquished&&!const_cast<Plant*>(p)->NotOnGround()){
  const int x=p->mX,y=VisualY(p);
  if(s.id==500){
   g->SetColor(Sexy::Color(57,37,18));g->FillRect(x+12,y+77,56,6);
   g->SetColor(s.phase==1?Sexy::Color(210,72,43):Sexy::Color(226,167,64));g->FillRect(x+13,y+78,s.phase==1?s.remaining*54/MemeShooterRules::BurstCount:s.heat*54/MemeShooterRules::MaxRage,4);
   if(s.phase==1){const int age=s.age%30;Puff(g,x+45,y+15-age,age,130);}
  }
 }
}
void Card(Sexy::Graphics* g,int x,int y,int id){const auto* d=Find(id);if(!d)return;
 DrawSeedPacket(g,x,y,static_cast<SeedType>(d->base),SEED_NONE,0,255,false,false);
 Sexy::Graphics bottom(*g);bottom.SetClipRect(x+3,y+52,44,17);PvzpDrawImageCelScaledF(&bottom,Sexy::IMAGE_SEEDS,x,y,2,0,1,1);
 PvzpDrawString(g,std::to_string(d->cost),x+23,y+65,Sexy::FONT_BRIANNETOD12,Sexy::Color(75,51,20),DS_ALIGN_CENTER);
}
void OnFired(Plant* p,Projectile* shot){
 if(Type(p)==ShooterPea){
  AbstractRigVisuals::Scope pose(p);
  float x=58,y=34;auto* pea=Sexy::IMAGE_PROJECTILEPEA;
  if(pea)SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(p->mHeadReanimID),"anim_face",pea->mWidth,pea->mHeight,pea->mWidth*.88f,pea->mHeight*.5f,x,y);
  shot->mPosX=p->mX+x-12;shot->mPosY=p->mY+y-12-shot->mPosZ;
  shot->mX=int(shot->mPosX);shot->mY=int(shot->mPosY+shot->mPosZ);
  shot->mMotionType=MOTION_STAR;shot->mVelX=3.33f;shot->mVelY=0;
  shotStyles[shot]=ShooterProjectile;return;
 }
 if(Type(p)==500){
 AbstractRigVisuals::Scope pose(p); // The actual projectile exits the posed mouth.
 float x,y;if(SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(p->mHeadReanimID),"idle_mouth",35,49,32,24.5f,x,y)){shot->mPosX=p->mX+x-12;shot->mPosY=p->mY+y-12-shot->mPosZ;shot->mX=int(shot->mPosX);shot->mY=int(shot->mPosY+shot->mPosZ);}
 const auto& s=states.at(p);shot->mMotionType=MOTION_STAR;
 if(s.phase==1){
  shotStyles[shot]=9;const float angle=MemeShooterRules::SpreadAngle(Sexy::Rand(1001)),speed=MemeShooterRules::BurstSpeed(Sexy::Rand(61));
  shot->mVelX=speed*std::cos(angle);shot->mVelY=speed*std::sin(angle);
 }else{
  const bool hit=MemeShooterRules::NormalStyle(Sexy::Rand(10))==10;
  const int style=hit?10:MemeShooterRules::FloatingFirst+Sexy::Rand(256);
  shotStyles[shot]=style;shot->mVelX=3.33f;shot->mVelY=hit?0:MemeShooterRules::FloatingStep(style,0);
 }
}}
int ShotStyle(const Projectile* shot){const auto it=shotStyles.find(shot);return it==shotStyles.end()?0:it->second;}
bool CanHit(const Projectile* shot){return ShotStyle(shot)==ShooterProjectile||MemeShooterRules::CanHit(BaseShotStyle(ShotStyle(shot)));}
void OnImpact(Projectile*,Zombie*){}
bool RestoreShotStyle(const Projectile* shot,int style){
 // Keep the new projectile distinct from retired self-thrower save records.
 if(style<0||(style>20&&style!=ShooterProjectile&&!MemeShooterRules::IsFloating(style))||style==19||shot->mDead)return false;
 if(style&&(shot->mMotionType!=MOTION_STAR||(shot->mProjectileType!=PROJECTILE_PEA&&shot->mProjectileType!=PROJECTILE_SNOWPEA&&shot->mProjectileType!=PROJECTILE_FIREBALL)))return false;
 if(style)shotStyles[shot]=style;else shotStyles.erase(shot);return true;
}
void ForgetShot(const Projectile* shot){shotStyles.erase(shot);}
void UpdateShot(Projectile* shot){
 const int style=BaseShotStyle(ShotStyle(shot));if(!style||shot->mDead||shot->mBoard->mPaused)return;
 if(shot->mPosY+shot->mPosZ<-40||shot->mPosY+shot->mPosZ>640){shot->Die();return;}
 if(MemeShooterRules::IsFloating(style))shot->mVelY=MemeShooterRules::FloatingStep(style,shot->mProjectileAge);
 else if(style<9)shot->mVelY=MemeShooterRules::WobbleStep(style,shot->mProjectileAge);
 else if(style>=11&&style<=18)shot->mVelY=MemeShooterRules::MissStep(style,shot->mProjectileAge);
}
}
