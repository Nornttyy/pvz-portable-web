// Individual character mechanics using the native rigs and seed bank.
#include "MemeCharacters.h"
#include "SandboxFactions.h"
#include "CoinPlants.h"
#include "StinkShroom.h"
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
std::map<const Projectile*,int> shotStyles; // 1..8 legacy wobble; 9 legacy burst; 10 hit; 11..18/20 legacy misses; 32..287 floating seeds; 304..309 row-bounded bursts.
void Burst(State& s){
 s.phase=1;s.heat=0;s.remaining=MemeShooterRules::BurstCount;s.delay=MemeShooterRules::BurstInterval(0);s.timer=0;s.pulse=30;
 gLawnApp->PlayRageRelease(); // Once per release, never once per pea or save restore.
}
bool Enemy(Plant* p,Zombie* z){return !z->mDead&&z->IsOnBoard()&&SandboxFactions::Enemy(p,z)&&!z->IsDeadOrDying()&&z->mHasHead&&!SandboxZombies::IsHeld(z);}
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
bool Hiding(const Plant* p){auto it=states.find(p);return it!=states.end()&&it->second.id==TuckingSunflower&&it->second.phase==1&&!p->mDead&&!p->mSquished&&!p->mIsAsleep&&p->mPlantHealth>0&&!const_cast<Plant*>(p)->NotOnGround();}
// The native production guard runs before advancing the sun countdown.
// Keep the remaining time while tucked, so standing up resumes, not restarts.
bool Producing(const Plant* p){return Type(p)==TuckingSunflower&&!Hiding(p);}
bool IsPuff(const Plant* p){return p&&(int(p->mSeedType)==8||(p->mSeedType==SEED_IMITATER&&int(p->mImitaterType)==8));}
int PuffCount(Board* b,int col,int row){
 int count=0;for(auto* p:b->mPlants)if(!p->mDead&&!p->NotOnGround()&&p->mPlantCol==col&&p->mRow==row&&IsPuff(p))++count;return count;
}
bool PuffMuzzle(const Plant* p,float& x,float& y){
 if(Type(p)!=TinyPuff)return false;
 AbstractRigVisuals::Scope pose(p);
 auto* tip=SandboxArt::NativeImage("PuffShroom_tip.png");
 return tip&&SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(p->mBodyReanimID),"PuffShroom_tip",tip->mWidth,tip->mHeight,tip->mWidth*.92f,tip->mHeight*.5f,x,y);
}
void Reset(){states.clear();shotStyles.clear();CoinPlants::Reset();}
void Forget(Plant* p){states.erase(p);CoinPlants::Forget(p);}
void Assign(Plant* p,int id){const auto* d=Find(id);if(!d||int(p->mSeedType)!=d->base)return;
 if(id==NukeShroom||id==IceChili){State s;s.id=id;s.health=p->mPlantHealth;s.delay=0;states[p]=s;return;}
 if(id==StinkShroom){State s;s.id=id;s.health=p->mPlantHealth;s.delay=0;states[p]=s;p->mLaunchRate=p->mLaunchCounter=StinkShroomRules::Interval;return;}
 if(id==TinyPuff){
  if(Type(p)==id)return;
  std::array<bool,TinyPuffRules::Limit> used{};
  if(p->mBoard)for(auto* other:p->mBoard->mPlants)if(other!=p&&!other->mDead&&other->mPlantCol==p->mPlantCol&&other->mRow==p->mRow&&Type(other)==id){const int slot=Data(other,3);if(slot>=0&&slot<TinyPuffRules::Limit)used[slot]=true;}
  int slot=0;while(slot<TinyPuffRules::Limit-1&&used[slot])++slot;
  State s;s.id=id;s.health=p->mPlantHealth;s.delay=0;s.direction=slot;states[p]=s;
  if(p->mBoard){p->mX=p->mBoard->GridToPixelX(p->mPlantCol,p->mRow);p->mY=p->mBoard->GridToPixelY(p->mPlantCol,p->mRow);}
  return; // Native short-range shooting, sleep and attack countdown stay intact.
 }
 State s;s.id=id;s.health=p->mPlantHealth;s.timer=0;if(id==500)s.direction=2;if(id==GatlingShooter)s.delay=GatlingInterval;if(id==CactusPalm)s.delay=PalmInterval;if(id==EverythingShooter)s.delay=EverythingShooterRules::Interval;states[p]=s;
 if(id==TuckingSunflower){states[p].delay=0;states[p].direction=3;return;}
 p->mLaunchCounter=9999;p->mShootingCounter=0;
}
std::array<int,10> Save(const Plant* p){auto it=states.find(p);if(it==states.end())return {};const auto& s=it->second;return {s.id,s.health,s.phase,s.heat,s.timer,s.delay,s.age,s.pulse,s.remaining,s.direction};}
bool Restore(Plant* p,const std::array<int,10>& a){
 if(a[0]==NukeShroom||a[0]==StinkShroom||a[0]==IceChili){
  if(int(p->mSeedType)!=Find(a[0])->base||p->mDead||a[1]<0||a[1]>p->mPlantMaxHealth||a[2]||a[3]||a[4]||a[5]||a[6]<0||a[6]>=1000000||a[7]||a[8]||a[9]!=1)return false;
  if(a[0]==StinkShroom)p->mLaunchRate=StinkShroomRules::Interval;
  states[p]={a[0],a[1],0,0,0,0,a[6],0,0,1};return true;
 }
 if(a[0]==TinyPuff){
  if(int(p->mSeedType)!=8||p->mDead||a[1]<0||a[1]>p->mPlantMaxHealth||a[2]||a[3]||a[4]||a[5]||a[6]||a[7]||a[8]||a[9]<0||a[9]>=TinyPuffRules::Limit)return false;
  states[p]={a[0],a[1],0,0,0,0,0,0,0,a[9]};return true;
 }
 if(a[0]==CactusPalm){
  if(int(p->mSeedType)!=26||p->mDead||a[1]<0||a[1]>p->mPlantMaxHealth||a[2]||a[3]||a[4]||a[5]<0||a[5]>PalmInterval||a[6]<0||a[6]>=1000000||a[7]||a[8]||a[9]!=1)return false;
  states[p]={a[0],a[1],0,0,0,a[5],a[6],0,0,1};p->mLaunchCounter=9999;
  // Native rig/save owns the recovery animation. Never restart its shot.
  p->mShootingCounter=std::min(p->mShootingCounter,1);return true;
 }
 if(a[0]==TuckingSunflower){
  if((int(p->mSeedType)!=Find(a[0])->base&&int(p->mSeedType)!=53)||p->mDead||a[1]<0||a[1]>p->mPlantMaxHealth||a[2]<0||a[2]>1||a[6]<0||a[6]>=1000000)return false;
  if(a[9]==1){
   // Old embarrassment/gaze saves become this card, without changing health,
   // sun countdown, planted position or the player's campaign progress.
   if(a[3]<0||a[3]>54||a[4]<0||a[4]>400||a[5]<0||a[5]>400||a[7]<0||a[7]>1||a[8]<0||a[8]>8||a[2]!=(a[4]>0)||((a[3]==0)!=(a[5]==0)))return false;
   states[p]={a[0],a[1],0,0,0,0,a[6],0,0,3};
  }else{
   if(a[9]!=3||a[3]||a[4]||a[5]||a[7]||a[8])return false;
   states[p]={a[0],a[1],a[2],0,0,0,a[6],0,0,3};
  }
  p->mSeedType=SEED_SUNFLOWER;
  return true; // Native save owns the production countdown.
 }
 if(a[0]==LongRepeater){
  if(int(p->mSeedType)!=Find(a[0])->base||p->mDead||a[1]<0||a[1]>p->mPlantMaxHealth||a[2]<0||a[2]>1||a[3]||a[4]||a[5]<0||a[5]>RepeaterRest||a[6]<0||a[6]>=1000000||a[7]||a[8]<0||a[8]>RepeaterCount||a[9]!=1||bool(a[2])!=bool(a[8]))return false;
  states[p]={a[0],a[1],a[2],0,0,a[5],a[6],0,a[8],1};
  p->mLaunchCounter=9999;p->mShootingCounter=0;return true;
 }
 if(a[0]==GatlingShooter){
  if(int(p->mSeedType)!=Find(a[0])->base||p->mDead||a[1]<0||a[1]>p->mPlantMaxHealth||a[2]<0||a[2]>1||a[3]<0||a[3]>GatlingHeatLimit||a[4]<0||a[4]>GatlingCooldown||a[5]<0||a[5]>GatlingInterval||a[6]<0||a[6]>=1000000||a[7]||a[8]||a[9]!=1)return false;
  if(a[2]?(a[3]!=GatlingHeatLimit||!a[4]||a[5]):(a[3]>=GatlingHeatLimit||a[4]))return false;
  states[p]={a[0],a[1],a[2],a[3],a[4],a[5],a[6],0,0,1};
  p->mLaunchCounter=9999;p->mShootingCounter=0;return true;
 }
 const bool shooter=a[0]==500,newShooter=shooter&&a[9]==2;
 const auto* d=Find(a[0]);if(!d||int(p->mSeedType)!=d->base||p->mDead||a[1]<0||a[1]>p->mPlantMaxHealth||a[2]<0||a[2]>(newShooter?1:2)||a[3]<0||a[3]>(newShooter?300:1000)||a[4]<0||a[4]>2000||a[5]<0||a[5]>2000||a[6]<0||a[6]>=1000000||a[7]<0||a[7]>50||a[8]<0||a[8]>(newShooter?150:3)||(!newShooter&&a[9]!=1&&a[9]!=-1))return false;
 if(newShooter&&((a[2]==1&&(a[8]==0||a[3]!=0))||(a[2]==0&&a[8]!=0)))return false;
 State s{a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9]};
 // Direction was unused for this character: 2 versions the new burst state.
 // Migrate old overheating saves without healing or inventing a free volley.
 if(shooter&&!newShooter){s.phase=0;s.heat=a[2]==0?a[3]*300/1000:0;s.timer=0;s.delay=std::min(a[5],150);s.remaining=0;s.pulse=0;s.direction=2;}
 if(shooter)s.heat=std::min(s.heat,MemeShooterRules::MaxRage); // Keep old 300-rage saves readable.
 if(shooter)s.remaining=std::min(s.remaining,MemeShooterRules::BurstCount); // Cap old 50/80/150-pea saves, never refill a volley.
 states[p]=s;p->mLaunchCounter=9999;p->mShootingCounter=0;return true;
}
int Data(const Plant* p,int field){const auto it=states.find(p);if(it==states.end())return -1;const auto& s=it->second;if(field==5)return s.id==TinyPuff?s.direction:-1;return field==0?s.phase:field==1?s.heat:field==2?s.timer:field==3?s.direction:s.remaining;}
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
  SandboxFactions::Scope faction(p);
  if(SandboxFactions::Frozen(p))continue;
  auto it=states.find(p);if(it==states.end())continue;if(p->mDead){states.erase(it);continue;}auto& s=it->second;
  // Hiding lets zombies chew the pad underneath, but does not make the
  // sunflower aquatic. Also repair unsupported flowers restored from saves.
  if((s.id==TuckingSunflower||s.id==TinyPuff)&&p->IsInPlay()&&!p->NotOnGround()&&b->IsPoolSquare(p->mPlantCol,p->mRow)){
   bool supported=false;
   for(auto* pad:b->mPlants){
    if(pad->mDead||pad->NotOnGround()||pad->mPlantHealth<=0||pad->mPlantCol!=p->mPlantCol||pad->mRow!=p->mRow)continue;
    const auto type=pad->mSeedType==SEED_IMITATER?pad->mImitaterType:pad->mSeedType;
    if(type==SEED_LILYPAD){supported=true;break;}
   }
   if(!supported){p->Die();continue;} // Die erases s via SandboxPlants::Forget.
  }
  if(s.id==TinyPuff){s.health=p->mPlantHealth;continue;}
  if(CoinPlantRules::IsPlant(s.id)){
   p->mLaunchCounter=9999;p->mShootingCounter=0;
   if(::StinkShroom::WorkTick(p))CoinPlants::Update(p);
   s.health=p->mPlantHealth;continue;
  }
  if(s.id==NukeShroom||s.id==StinkShroom||s.id==IceChili){s.health=p->mPlantHealth;if(!p->mIsAsleep)s.age=(s.age+1)%1000000;continue;}
  if(s.id!=TuckingSunflower){p->mLaunchCounter=9999;if(s.id!=CactusPalm)p->mShootingCounter=0;}
  if(p->mIsAsleep||p->mSquished||p->NotOnGround()||p->mPlantHealth<=0){if(s.id==TuckingSunflower)s.phase=0;continue;}
  if(s.id!=TuckingSunflower&&s.id!=501&&!::StinkShroom::WorkTick(p)){s.health=p->mPlantHealth;continue;}
  s.age=(s.age+1)%1000000;if(s.pulse)--s.pulse;if(s.delay)--s.delay;if(s.timer)--s.timer;
  if(s.id==TuckingSunflower){
   bool danger=false;
   // Use native attack rectangles (including diggers/mirrored walkers), not
   // sprite origins. A wider release boundary prevents edge flicker.
   const int range=s.phase?84:60;
   for(auto* z:b->mZombies){
    if(z->mDead||!z->IsOnBoard()||!SandboxFactions::Enemy(p,z)||z->IsDeadOrDying()||z->IsFlying()||z->mRow!=p->mRow)continue;
    const auto rect=z->GetZombieAttackRect();
    if(rect.mWidth>0&&rect.mX<=p->mX+70+range&&rect.mX+rect.mWidth>=p->mX+10-range){danger=true;break;}
   }
   s.phase=danger?1:0;
  }else if(s.id==500){
   if(s.phase==0&&s.heat>=MemeShooterRules::MaxRage)Burst(s);
   const bool room=b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8;
   if(s.phase==1){
    if(!s.delay&&room){
     Shoot(p,nullptr);--s.remaining;s.delay=MemeShooterRules::BurstInterval(MemeShooterRules::BurstCount-s.remaining);s.pulse=10;
     if(!s.remaining){s.phase=0;s.heat=0;s.delay=MemeShooterRules::RecoveryDelay;}
    }
   }else if(!s.delay&&room){
    if(auto* target=p->FindTargetZombie(p->mRow,WEAPON_PRIMARY);target||SandboxFactions::Target(p,p->mRow)){
     Shoot(p,target);s.heat+=MemeShooterRules::PerShot;s.delay=MemeShooterRules::NormalDelay;
     if(s.heat>=MemeShooterRules::MaxRage)Burst(s);
    }
   }
  }else if(s.id==CactusPalm){
   if(!s.delay&&(p->mState==STATE_CACTUS_LOW||p->mState==STATE_CACTUS_HIGH)&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8){
    const bool high=p->mState==STATE_CACTUS_HIGH;const auto weapon=high?WEAPON_PRIMARY:WEAPON_SECONDARY;
    if(auto* target=p->FindTargetZombie(p->mRow,weapon);target||SandboxFactions::Target(p,p->mRow,weapon)){
     p->Fire(target,p->mRow,weapon);s.delay=PalmInterval;
     p->PlayBodyReanim(high?"anim_shootinghigh":"anim_shooting",REANIM_PLAY_ONCE_AND_HOLD,3,35);
     // 1 only runs native recovery; it cannot fire a second projectile.
     p->mShootingCounter=1;
    }
   }
  }else if(s.id==GatlingShooter){
   if(s.phase&&!s.timer){s.phase=0;s.heat=0;s.delay=0;}
   if(!s.phase&&!s.delay&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8){
    if(auto* target=p->FindTargetZombie(p->mRow,WEAPON_PRIMARY);target||SandboxFactions::Target(p,p->mRow)){
     // Reuse the native multi-barrel recoil, without restarting it every pea.
     if(s.heat%10==0)Shoot(p,target);else p->Fire(target,p->mRow,WEAPON_PRIMARY);
     ++s.heat;s.delay=GatlingInterval;
     if(s.heat==GatlingHeatLimit){s.phase=1;s.timer=GatlingCooldown;s.delay=0;}
    }
   }
  }else if(s.id==LongRepeater){
   const bool room=b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8;
   if(!s.phase&&!s.delay&&room&&SandboxFactions::HasTarget(p,p->mRow)){s.phase=1;s.remaining=RepeaterCount;}
   if(s.phase&&!s.delay&&room){
    // Keep one committed volley even if its first target dies. Recoil cycles
    // normally, rather than resetting the head animation every two ticks.
    if((RepeaterCount-s.remaining)%10==0)Shoot(p,nullptr);else p->Fire(nullptr,p->mRow,WEAPON_PRIMARY);
    --s.remaining;s.delay=RepeaterInterval;
    if(!s.remaining){s.phase=0;s.delay=RepeaterRest;}
   }
  }else if(s.id==ShooterPea||s.id==EverythingShooter){
   if(!s.delay&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8){
    if(auto* target=p->FindTargetZombie(p->mRow,WEAPON_PRIMARY);target||SandboxFactions::Target(p,p->mRow)){Shoot(p,target);s.delay=150;s.pulse=22;}
   }
  }else if(s.id==501){
   if(!s.timer&&p->mRecentlyEatenCountdown>0){
    Zombie* target=nullptr;
    for(auto* z:b->mZombies)if(Enemy(p,z)&&z->mRow==p->mRow&&z->mIsEating&&SandboxFactions::NutContact(p,z)&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))&&(!target||z->mPosX>target->mPosX))target=z;
    if(target){s.timer=300;s.pulse=50;s.phase=1;s.heat=int(target->mPosX-p->mX+65);gLawnApp->PlayFoley(FOLEY_THROW);}
   }
   if(s.phase==1&&s.pulse==28){
    for(auto* z:b->mZombies)if(Enemy(p,z)&&z->mRow==p->mRow&&SandboxFactions::NutContact(p,z,40)&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))){
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
 if(s.id==TinyPuff){const auto pose=TinyPuffRules::At(s.direction);x+=sx*(pose.x-TinyPuffRules::AnchorX*TinyPuffRules::Scale);y+=sy*(pose.y-TinyPuffRules::AnchorY*TinyPuffRules::Scale);sx*=TinyPuffRules::Scale;sy*=TinyPuffRules::Scale;return;}
 float horizontal=1,vertical=1;
 if(s.id==500&&s.phase==1){const float recoil=std::sin(s.age*0.16f)*0.04f;horizontal+=0.10f+recoil;vertical-=0.07f+recoil;x-=sx*(4+6*(1-float(s.remaining)/MemeShooterRules::BurstCount));}
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
   if(s.phase==1){const int age=s.age%30;Puff(g,x+45,y+15-age,age,130);}
  }
  if(s.id==GatlingShooter){
   if(s.phase){const int age=s.age%30;Puff(g,x+56,y+12-age,age,120);}
  }
 }
}
void Card(Sexy::Graphics* g,int x,int y,int id){const auto* d=Find(id);if(!d)return;
 DrawSeedPacket(g,x,y,static_cast<SeedType>(d->base),SEED_NONE,0,255,false,false);
 Sexy::Graphics bottom(*g);bottom.SetClipRect(x+3,y+52,44,17);PvzpDrawImageCelScaledF(&bottom,Sexy::IMAGE_SEEDS,x,y,2,0,1,1);
 PvzpDrawString(g,std::to_string(d->cost),x+23,y+65,Sexy::FONT_BRIANNETOD12,Sexy::Color(75,51,20),DS_ALIGN_CENTER);
}
void OnFired(Plant* p,Projectile* shot){
 if(Type(p)==TinyPuff){
  shotStyles[shot]=TinyPuffProjectile;
  float x,y;if(PuffMuzzle(p,x,y)){shot->mPosX=p->mX+x-12;shot->mPosY=p->mY+y-12-shot->mPosZ;shot->mX=int(shot->mPosX);shot->mY=int(shot->mPosY+shot->mPosZ);}
  return;
 }
 if(Type(p)==CactusPalm){
  shotStyles[shot]=Sexy::Rand(100)<20?CriticalPalmProjectile:PalmProjectile;
  Sexy::SexyTransform2D palm;
  if(SandboxArt::PalmMatrix(gLawnApp->ReanimationTryToGet(p->mBodyReanimID),palm)){
   shot->mPosX=p->mX+palm.m02-12;shot->mPosY=p->mY+palm.m12-12-shot->mPosZ;
   shot->mX=int(shot->mPosX);shot->mY=int(shot->mPosY+shot->mPosZ);
  }
  return;
 }
 if(Type(p)==GatlingShooter){shotStyles[shot]=GatlingProjectile;return;}
 if(Type(p)==LongRepeater){shotStyles[shot]=WeakProjectile;return;}
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
  shotStyles[shot]=MemeShooterRules::BurstStyle(p->mRow);const float angle=MemeShooterRules::SpreadAngle(Sexy::Rand(1001)),speed=MemeShooterRules::BurstSpeed(Sexy::Rand(61));
  shot->mVelX=speed*std::cos(angle);shot->mVelY=speed*std::sin(angle);
 }else{
  const bool hit=MemeShooterRules::NormalStyle(Sexy::Rand(10))==10;
  const int style=hit?10:MemeShooterRules::FloatingFirst+Sexy::Rand(256);
  shotStyles[shot]=style;shot->mVelX=3.33f;shot->mVelY=hit?0:MemeShooterRules::FloatingStep(style,0);
 }
}}
int ShotStyle(const Projectile* shot){const auto it=shotStyles.find(shot);return it==shotStyles.end()?0:it->second;}
bool CanHit(const Projectile* shot){return IsStraightShot(ShotStyle(shot))||ShotStyle(shot)==ShooterProjectile||MemeShooterRules::CanHit(BaseShotStyle(ShotStyle(shot)));}
bool CanHitRow(const Projectile* shot,int row){return MemeShooterRules::CanHitRow(BaseShotStyle(ShotStyle(shot)),row);}
void OnImpact(Projectile*,Zombie*){}
bool RestoreShotStyle(const Projectile* shot,int style){
 if(CoinPlantRules::Shot(style)){
  if(shot->mDead||shot->mProjectileType!=PROJECTILE_SPIKE||shot->mMotionType!=MOTION_STRAIGHT)return false;
  shotStyles[shot]=style;return true;
 }
 if(EverythingShooterRules::Own(style)){
  if(shot->mDead||!EverythingShooterRules::Valid(style,int(shot->mProjectileType),int(shot->mMotionType)))return false;
  shotStyles[shot]=style;return true;
 }
 if(style==TinyPuffProjectile){if(shot->mDead||shot->mProjectileType!=PROJECTILE_PUFF||shot->mMotionType!=MOTION_PUFF)return false;shotStyles[shot]=style;return true;}
 // Keep the new projectile distinct from retired self-thrower save records.
 if(style<0||(style>20&&style!=ShooterProjectile&&!IsStraightShot(style)&&!MemeShooterRules::IsFloating(style)&&!MemeShooterRules::IsBoundedBurst(style))||style==19||shot->mDead)return false;
 if(style&&(shot->mMotionType!=(IsStraightShot(style)?MOTION_STRAIGHT:MOTION_STAR)||(IsPalmShot(style)?shot->mProjectileType!=PROJECTILE_SPIKE:(shot->mProjectileType!=PROJECTILE_PEA&&shot->mProjectileType!=PROJECTILE_SNOWPEA&&shot->mProjectileType!=PROJECTILE_FIREBALL))))return false;
 // Legacy bursts did not save a firing row; anchor them to the loaded lane.
 if(style==9)style=MemeShooterRules::BurstStyle(std::clamp(shot->mRow,0,5));
 if(style)shotStyles[shot]=style;else shotStyles.erase(shot);return true;
}
void ForgetShot(const Projectile* shot){shotStyles.erase(shot);}
void UpdateShot(Projectile* shot){
 const int style=BaseShotStyle(ShotStyle(shot));if(!style||shot->mDead||shot->mBoard->mPaused)return;
 if(EverythingShooterRules::Own(style))return; // Native lob/cob paths may go above the screen.
 if(shot->mPosY+shot->mPosZ<-40||shot->mPosY+shot->mPosZ>640){shot->Die();return;}
 if(MemeShooterRules::IsBoundedBurst(style)){
  const int nextRow=shot->mBoard->PixelToGridYKeepOnBoard(int(shot->mPosX+shot->mVelX),int(shot->mPosY+shot->mVelY));
  if(!MemeShooterRules::CanHitRow(style,nextRow)){shot->Die();return;}
 }
 if(MemeShooterRules::IsFloating(style))shot->mVelY=MemeShooterRules::FloatingStep(style,shot->mProjectileAge);
 else if(style<9)shot->mVelY=MemeShooterRules::WobbleStep(style,shot->mProjectileAge);
 else if(style>=11&&style<=18)shot->mVelY=MemeShooterRules::MissStep(style,shot->mProjectileAge);
}
}
