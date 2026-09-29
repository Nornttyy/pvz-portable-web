// Four fixed characters. No power tokens, fusion recipes or shared stat variants.
#include "MemeCharacters.h"
#include "SandboxPlants.h"
#include "SandboxArt.h"
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
#include <map>
#include <algorithm>
#include <cmath>
#include <string>
namespace MemeCharacters {
namespace {
// Saved in the existing ten-integer optional plant record.
struct State {int id=0,health=0,phase=0,heat=0,timer=0,delay=50,age=0,pulse=0,remaining=0,direction=1;};
std::map<const Plant*,State> states;
// Native zombie IDs are stable through DataArray recycling; never retain pointers.
std::map<unsigned,int> laneCooldown;
bool Enemy(Zombie* z){return !z->mDead&&z->IsOnBoard()&&!z->mMindControlled&&!z->IsDeadOrDying()&&z->mHasHead;}
bool Walker(Zombie* z){const int type=int(z->mZombieType);return Enemy(z)&&(type==0||type==1||type==2||type==4||type==5||type==6||type==7||type==24)&&z->mZombiePhase==PHASE_ZOMBIE_NORMAL&&z->mZombieHeight==HEIGHT_ZOMBIE_NORMAL&&!z->mInPool;}
bool Lane(Board* b,int from,int to){return to>=0&&to<(b->StageHasPool()?6:5)&&b->RowCanHaveZombies(to)&&!(b->StageHasPool()&&(from==2||from==3||to==2||to==3));}
bool ReadyToMove(Board* b,Zombie* z){return Walker(z)&&!laneCooldown.contains(unsigned(b->ZombieGetID(z)));}
void Move(Board* b,Zombie* z,int row){
 z->SetRow(row); // Native UpdateZombiePosition smoothly walks to the new lane.
 laneCooldown[unsigned(b->ZombieGetID(z))]=350;
}
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
bool Hiding(const Plant* p){auto it=states.find(p);return it!=states.end()&&it->second.id==503&&it->second.phase==1&&!p->mIsAsleep&&!p->mSquished;}
void Reset(){states.clear();laneCooldown.clear();}
void Forget(Plant* p){states.erase(p);}
void Assign(Plant* p,int id){const auto* d=Find(id);if(!d||int(p->mSeedType)!=d->base)return;
 State s;s.id=id;s.health=p->mPlantHealth;s.timer=id==502?180:0;states[p]=s;
 p->mLaunchCounter=9999;p->mShootingCounter=0;
 if(id==502)p->SetSleeping(false); // This character is a daytime lure, not a puff shooter.
}
std::array<int,10> Save(const Plant* p){auto it=states.find(p);if(it==states.end())return {};const auto& s=it->second;return {s.id,s.health,s.phase,s.heat,s.timer,s.delay,s.age,s.pulse,s.remaining,s.direction};}
bool Restore(Plant* p,const std::array<int,10>& a){
 const auto* d=Find(a[0]);if(!d||int(p->mSeedType)!=d->base||p->mDead||a[1]<0||a[1]>p->mPlantMaxHealth||a[2]<0||a[2]>2||a[3]<0||a[3]>1000||a[4]<0||a[4]>2000||a[5]<0||a[5]>2000||a[6]<0||a[6]>=1000000||a[7]<0||a[7]>50||a[8]<0||a[8]>3||(a[9]!=1&&a[9]!=-1))return false;
 states[p]={a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9]};p->mLaunchCounter=9999;p->mShootingCounter=0;return true;
}
int Data(const Plant* p,int field){const auto it=states.find(p);if(it==states.end())return -1;const auto& s=it->second;return field==0?s.phase:field==1?s.heat:field==2?s.timer:field==3?s.direction:s.remaining;}
bool Activate(Plant* p,int direction){
 auto it=states.find(p);if(it==states.end()||p->mDead||p->mBoard->mPaused||p->mIsAsleep||p->mSquished||p->NotOnGround()||p->mPlantHealth<=0)return false;
 auto& s=it->second;auto* b=p->mBoard;
 if(s.id==500){
  if(s.phase==2||s.heat<300)return false;
  const int distance=40+s.heat/15;
  for(auto* z:b->mZombies)if(Walker(z)&&z->mRow==p->mRow&&z->mPosX>p->mX-20&&z->mPosX<p->mX+230){z->mPosX=std::min(850.0f,z->mPosX+distance);z->UpdateReanim();}
  s.heat=0;s.delay=90;s.pulse=40;gLawnApp->PlayFoley(FOLEY_THROW);return true;
 }
 if(s.id==501){
  if(direction)s.direction=direction<0?-1:1;
  if(s.timer||!Lane(b,p->mRow,p->mRow+s.direction))return true;
  Zombie* closest=nullptr;
  for(auto* z:b->mZombies)if(ReadyToMove(b,z)&&z->mRow==p->mRow&&z->mPosX>p->mX-35&&z->mPosX<p->mX+150&&(!closest||z->mPosX<closest->mPosX))closest=z;
  if(closest){Move(b,closest,p->mRow+s.direction);s.timer=600;s.pulse=40;gLawnApp->PlayFoley(FOLEY_THROW);}
  return true;
 }
 return false;
}
bool Click(Board* b,int x,int y){
 for(auto* p:b->mPlants)if(!p->mDead&&Is(p)&&x>=p->mX&&x<p->mX+80&&y>=p->mY&&y<p->mY+80){
  if(Type(p)==500||Type(p)==501){Activate(p,y<p->mY+40?-1:1);return true;}
 }return false;
}
void Tick(Board* b){
 if(b->mPaused)return;
 for(auto it=laneCooldown.begin();it!=laneCooldown.end();)if(--it->second<=0||!b->ZombieTryToGet(static_cast<ZombieID>(it->first)))it=laneCooldown.erase(it);else ++it;
 for(auto* p:b->mPlants){
  auto it=states.find(p);if(it==states.end())continue;if(p->mDead){states.erase(it);continue;}auto& s=it->second;
  p->mLaunchCounter=9999;p->mShootingCounter=0;
  if(p->mIsAsleep||p->mSquished||p->NotOnGround()||p->mPlantHealth<=0)continue;
  s.age=(s.age+1)%1000000;if(s.pulse)--s.pulse;if(s.delay)--s.delay;if(s.timer)--s.timer;
  if(s.id==500){
   if(s.phase==2){s.heat=s.timer*2;if(!s.timer){s.phase=0;s.heat=0;}continue;}
   auto* target=p->FindTargetZombie(p->mRow,WEAPON_PRIMARY);
   if(!target){s.heat=std::max(0,s.heat-1);continue;}
   if(!s.delay&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8){Shoot(p,target);s.heat=std::min(1000,s.heat+100);s.delay=std::max(55,110-s.heat/20);}
   if(s.heat>=1000){s.phase=2;s.timer=500;s.pulse=50;p->mPlantHealth-=120;p->mEatenFlashCountdown=20;if(p->mPlantHealth<=0){p->Die();continue;}}
  }else if(s.id==501){s.heat=(600-s.timer)*1000/600;
  }else if(s.id==502){
   s.heat=(500-std::min(500,s.timer))*2;
   if(!s.timer){
    Zombie* closest=nullptr;float distance=10000;
    for(auto* z:b->mZombies)if(ReadyToMove(b,z)&&std::abs(z->mRow-p->mRow)==1&&Lane(b,z->mRow,p->mRow)&&z->mPosX>p->mX+20&&z->mPosX<p->mX+310){const float d=std::abs(z->mPosX-p->mX);if(d<distance){closest=z;distance=d;}}
    if(closest){Move(b,closest,p->mRow);s.timer=500;s.pulse=50;gLawnApp->PlayFoley(FOLEY_THROW);}else s.timer=30;
   }
  }else if(s.id==503){
   Zombie *front=nullptr,*back=nullptr;
   for(auto* z:b->mZombies)if(Enemy(z)&&z->mRow==p->mRow&&z->EffectedByDamage(p->GetDamageRangeFlags(WEAPON_PRIMARY))){
    // Normal bite box is x+50..70. Hide just before contact and wake
    // after the whole bite box passes, not a full extra tile behind.
    if(z->mPosX>=p->mX-65&&z->mPosX<p->mX+45)front=z;
    if(z->mPosX<p->mX-65&&z->mPosX>p->mX-330&&(!back||z->mPosX>back->mPosX))back=z;
   }
   if(s.phase==0&&!s.timer&&front){s.phase=1;s.timer=1400;s.pulse=20;}
   if(s.phase==1&&back&&!front){s.phase=2;s.timer=180;s.remaining=3;s.delay=35;s.pulse=30;}
   else if(s.phase==1&&!s.timer){s.phase=0;s.timer=500;}
   if(s.phase==2){
    if(s.remaining&&!s.delay&&b->mProjectiles.mSize<b->mProjectiles.mMaxSize-8){
     auto* shot=b->AddProjectile(p->mX+12,p->mY+22,p->mRenderOrder-1,p->mRow,PROJECTILE_PEA);
     shot->mMotionType=MOTION_BACKWARDS;shot->mDamageRangeFlags=p->GetDamageRangeFlags(WEAPON_PRIMARY);SandboxPlants::RestoreShot(shot,300);
     --s.remaining;s.delay=16;gLawnApp->PlayFoley(FOLEY_THROW);
    }
    if(!s.timer){s.phase=0;s.timer=500;s.remaining=0;}
   }
   s.heat=s.phase==1?1000:s.phase==2?std::min(1000,s.timer*5):std::max(0,1000-s.timer*2);
  }
  s.health=p->mPlantHealth;
 }
}
void Tint(const Plant* p,Sexy::Color& c){auto it=states.find(p);if(it==states.end())return;const auto& s=it->second;
 if(s.id==500){c.mGreen=c.mGreen*(255-s.heat/7)/255;c.mBlue=c.mBlue*(235-s.heat/6)/255;}
 if(s.id==502)c.mBlue=c.mBlue*180/255;
 if(s.id==503){c.mRed=c.mRed*205/255;c.mGreen=c.mGreen*215/255;}
}
void Scale(const Plant* p,float& x,float& y,float& sx,float& sy){auto it=states.find(p);if(it==states.end()||p->mSquished)return;const auto& s=it->second;
 float horizontal=1,vertical=1;
 if(s.id==500){horizontal+=s.heat/16000.0f;vertical-=s.heat/20000.0f;}
 if(s.id==502&&s.pulse){vertical+=0.12f*std::sin(s.pulse*0.18f);horizontal-=0.06f*std::sin(s.pulse*0.18f);}
 if(s.id==503&&s.phase==1){vertical=0.28f+0.72f*s.pulse/20.0f;horizontal=1.15f;}
 if(s.id==503&&s.phase==2&&s.pulse)vertical=1.0f-0.72f*s.pulse/30.0f;
 x+=40*sx*(1-horizontal);y+=65*sy*(1-vertical);sx*=horizontal;sy*=vertical;
}
void Effects(Sexy::Graphics* g,Board* b,int row){
 for(const auto& [p,s]:states)if(!p->mDead&&p->mRow==row&&!p->mSquished){
  const int x=p->mX,y=p->mY;
  if(s.id==500){
   g->SetColor(Sexy::Color(57,37,18));g->FillRect(x+12,y+77,56,6);
   g->SetColor(s.heat>700?Sexy::Color(238,67,37):Sexy::Color(240,164,45));g->FillRect(x+13,y+78,s.heat*54/1000,4);
   if(s.heat>700||s.pulse){const int age=s.age%30;Puff(g,x+45,y+15-age,age,150);}
  }else if(s.id==501){
   const int cy=y+15;g->SetColor(Sexy::Color(250,227,103));g->DrawLine(x+60,cy+5,x+60,cy+22);
   const int tip=s.direction<0?cy+5:cy+22;g->DrawLine(x+60,tip,x+55,tip-s.direction*6);g->DrawLine(x+60,tip,x+65,tip-s.direction*6);
   if(s.timer){g->SetColor(Sexy::Color(96,71,39));g->FillRect(x+15,y+78,50,4);g->SetColor(Sexy::Color(235,205,83));g->FillRect(x+15,y+78,50*(600-s.timer)/600,4);}
  }else if(s.id==502){
   if(s.pulse){const int r=50-s.pulse;for(int sign:{-1,1})Puff(g,x+40,y+25+sign*r,r,130);}
   PvzpDrawString(g,"!",x+40,y+5,Sexy::FONT_BRIANNETOD12,Sexy::Color(248,202,74),DS_ALIGN_CENTER);
  }else if(s.id==503&&s.phase==2&&s.pulse)Puff(g,x+5,y+20,30-s.pulse,120);
 }
}
void Card(Sexy::Graphics* g,int x,int y,int id){const auto* d=Find(id);if(!d)return;
 DrawSeedPacket(g,x,y,static_cast<SeedType>(d->base),SEED_NONE,0,255,false,false);
 Sexy::Graphics bottom(*g);bottom.SetClipRect(x+3,y+52,44,17);PvzpDrawImageCelScaledF(&bottom,Sexy::IMAGE_SEEDS,x,y,2,0,1,1);
 PvzpDrawString(g,std::to_string(d->cost),x+23,y+65,Sexy::FONT_BRIANNETOD12,Sexy::Color(75,51,20),DS_ALIGN_CENTER);
 g->SetColor(Sexy::Color(230,238,196));g->FillRect(x+4,y+3,42,11);
 PvzpDrawString(g,d->shortName,x+25,y+12,Sexy::FONT_BRIANNETOD12,Sexy::Color(91,64,32),DS_ALIGN_CENTER);
}
void OnFired(Plant* p,Projectile* shot){if(Type(p)==500){
 float x,y;if(SandboxArt::TrackPoint(gLawnApp->ReanimationTryToGet(p->mHeadReanimID),"idle_mouth",35,49,32,24.5f,x,y)){shot->mPosX=p->mX+x-12;shot->mPosY=p->mY+y-12-shot->mPosZ;shot->mX=int(shot->mPosX);shot->mY=int(shot->mPosY+shot->mPosZ);}
}}
}
