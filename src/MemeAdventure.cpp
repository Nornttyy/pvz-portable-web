// Adventure's independent power slot; it never consumes a native seed-bank slot.
#include "MemeAdventure.h"
#include "MemeAdventureRules.h"
#include "Sandbox.h"
#include "SandboxFusion.h"
#include "SandboxButton.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Projectile.h"
#include "Lawn/CursorObject.h"
#include "Lawn/SeedPacket.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include "widget/WidgetManager.h"
#include <format>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif
namespace MemeAdventure {
namespace {
int power=180,cooldown=0,noticeTime=0;
bool choosing=false,selected=false;
std::string notice;
Save pending;
const char* Name(int p){return p==180?"红温":p==181?"内卷":"摆烂";}
bool Running(Board* b){return Visible(b)&&!b->mPaused&&!b->mTimeStopCounter&&!b->mLevelComplete&&!b->mLevelAwardSpawned&&b->mApp->GetDialogCount()==0;}
void Say(const std::string& value){notice=value;noticeTime=180;}
Plant* Target(Board* b,int x,int y){
 const int col=b->PixelToGridX(x,y),row=b->PixelToGridY(x,y);if(col<0||col>=9||row<0||row>=(b->StageHasPool()?6:5))return nullptr;
 Plant* found=nullptr;
 for(auto* p:b->mPlants)if(!p->mDead&&(p->mPlantCol==col||p->mSeedType==47&&p->mPlantCol+1==col)&&p->mRow==row&&!SandboxPlants::IsCustom(p)&&p->mPlantHealth>0&&!p->mSquished&&!p->NotOnGround()){
  const auto priority=[](Plant* plant){const int base=SandboxPlants::EffectiveBase(plant);return base==35?4:base==30?2:base==16||base==33?1:3;};
  if(!found||priority(p)>priority(found))found=p;
 }
 if(!found||found->mPlantHealth<=0||found->mSquished||found->NotOnGround()||SandboxPlants::IsCustom(found))return nullptr;
 return SandboxMemeRules::Result(SandboxPlants::EffectiveBase(found),power)?found:nullptr;
}
}
void Reset(){power=180;cooldown=0;noticeTime=0;choosing=selected=false;notice.clear();pending={};}
bool Visible(Board* b){return b&&!gSandboxEnabled&&b->mApp->IsAdventureMode()&&b->mApp->mGameScene==SCENE_PLAYING&&!b->HasConveyorBeltSeedBank()&&!b->mApp->IsChallengeWithoutSeedBank()&&!b->mApp->IsWallnutBowlingLevel()&&!b->mApp->IsScaryPotterLevel();}
bool Cancel(){const bool used=selected||choosing;selected=choosing=false;return used;}
void Tick(Board* b){
 if(!Running(b)){Cancel();return;}
 if(cooldown>0)--cooldown;if(noticeTime>0)--noticeTime;
 SandboxPlants::Tick(b);
}
bool MouseDown(Board* b,int x,int y,int clicks){
 using namespace MemeAdventureRules;
 if(!Running(b)){Cancel();return false;}
 if(clicks<0){return Cancel();}
 if(Slot.Contains(x,y)){
  if(b->mCursorObject->mCursorType!=CURSOR_TYPE_PLANT_FROM_BANK){b->RefreshSeedPacketFromCursor();b->ClearCursor();}
  selected=false;choosing=!choosing;b->mApp->PlaySample(Sexy::SOUND_SEEDLIFT);return true;
 }
 if(choosing){
  for(int i=0;i<3;++i)if(Choice(i).Contains(x,y)){
   const int candidate=180+i;
   if(!b->mApp->HasFinishedAdventure()&&b->mLevel<Unlock(candidate)){Say(std::format("{} 解锁",b->mApp->GetStageString(Unlock(candidate))));return true;}
   if(cooldown){Say("冷却中");return true;}
   if(!b->CanTakeSunMoney(Cost(candidate))){Say("阳光不足");b->mOutOfMoneyCounter=70;return true;}
   power=candidate;selected=true;choosing=false;b->mApp->PlaySample(Sexy::SOUND_SEEDLIFT);return true;
  }
  choosing=false;
 }
 if(!selected)return false;
 // Native cards/shovel/menu remain usable; leaving the lawn cancels selection.
 if(y<82){selected=false;return false;}
 // Collecting suns must remain possible while a power is in hand.
 HitResult hit;b->MouseHitTest(x,y,&hit);if(hit.mObjectType==OBJECT_TYPE_COIN)return false;
 if(b->mCursorObject->mCursorType==CURSOR_TYPE_PLANT_FROM_BANK){
  const int base=b->GetSeedTypeInCursor();
  if(!Unlocked(b->mLevel,b->mApp->HasFinishedAdventure(),power,base)){Say("力量尚未解锁");return true;}
  if(cooldown||!b->CanTakeSunMoney(Cost(power)+b->GetCurrentPlantCost(static_cast<SeedType>(base),SEED_NONE))){Say("阳光不足");return true;}
  return false; // Native planting validates terrain and spends the plant price first.
 }
 auto* p=Target(b,x,y);
 if(!p){Say("请选择可用的植物");return true;}
 if(!Unlocked(b->mLevel,b->mApp->HasFinishedAdventure(),power,SandboxPlants::EffectiveBase(p))){Say(std::format("{} 解锁",b->mApp->GetStageString(RequiredLevel(power,SandboxPlants::EffectiveBase(p)))));return true;}
 if(cooldown||!b->TakeSunMoney(Cost(power)))return true;
 SandboxPlants::Assign(p,SandboxMemeRules::Result(SandboxPlants::EffectiveBase(p),power));
 cooldown=Cooldown;selected=false;noticeTime=0;b->mApp->PlaySample(Sexy::SOUND_PLANTGROW);b->MarkAllDirty();return true;
}
void OnPlanted(Plant* p){
 auto* b=p->mBoard;if(!selected||!Running(b)||cooldown||!MemeAdventureRules::Unlocked(b->mLevel,b->mApp->HasFinishedAdventure(),power,SandboxPlants::EffectiveBase(p)))return;
 if(!b->TakeSunMoney(MemeAdventureRules::Cost(power)))return;
 SandboxPlants::Assign(p,SandboxMemeRules::Result(SandboxPlants::EffectiveBase(p),power));cooldown=MemeAdventureRules::Cooldown;selected=choosing=false;
 b->mApp->PlaySample(Sexy::SOUND_PLANTGROW);
}
void Draw(Board* b,Sexy::Graphics* graphics){
 if(!Visible(b)||b->mLevelComplete||b->mLevelAwardSpawned)return;
 using namespace Sexy;using namespace MemeAdventureRules;
 Graphics g(*graphics);const auto* wm=b->mApp->mWidgetManager.get();
 const int mx=wm->mLastMouseX-b->mX,my=wm->mLastMouseY-b->mY;
 // Compact native wood slot beneath the menu, clear of even a ten-card bank.
 g.DrawImage(IMAGE_SHOVELBANK,Slot.x,Slot.y,Slot.w,Slot.h);
 Graphics icon(g);icon.Translate(Slot.x+3,Slot.y+2);icon.mScaleX=0.46f;icon.mScaleY=0.46f;
 DrawSeedPacket(&icon,0,0,static_cast<SeedType>(power==180?20:power==181?35:9),SEED_NONE,0,255,false,false);
 const bool locked=!b->mApp->HasFinishedAdventure()&&b->mLevel<Unlock(power);
 PvzpDrawString(&g,Name(power),Slot.x+55,Slot.y+15,FONT_BRIANNETOD12,Color(255,225,140),DS_ALIGN_CENTER);
 const std::string sub=locked?b->mApp->GetStageString(Unlock(power)):cooldown?std::format("{}s",(cooldown+99)/100):std::to_string(Cost(power));
 PvzpDrawString(&g,sub,Slot.x+55,Slot.y+32,FONT_BRIANNETOD12,Color(255,225,140),DS_ALIGN_CENTER);
 if(cooldown||locked){g.SetColor(Color(0,0,0,90));g.FillRect(Slot.x+2,Slot.y+2,22,Slot.h-4);}
 if(selected||choosing){g.SetColor(Color(255,218,70));g.DrawRect(Slot.x,Slot.y,Slot.w-1,Slot.h-1);}
 if(choosing)for(int i=0;i<3;++i){
  const auto box=Choice(i);const int p=180+i;
  const bool open=b->mApp->HasFinishedAdventure()||b->mLevel>=Unlock(p);
  const std::string label=std::string(Name(p))+"  "+(open?std::to_string(Cost(p)):b->mApp->GetStageString(Unlock(p)));
  SandboxDrawButton(&g,box,label,false,box.Contains(mx,my));
 }
 if(selected){
  auto* target=Target(b,mx,my);if(target){const bool valid=Unlocked(b->mLevel,b->mApp->HasFinishedAdventure(),power,SandboxPlants::EffectiveBase(target));
   g.SetColor(valid?Color(100,230,70,90):Color(220,60,40,90));g.FillRect(target->mX,target->mY,80,b->StageHasPool()?85:100);
  }
  Graphics cursor(g);cursor.Translate(std::clamp(mx+12,0,765),std::clamp(my-45,0,544));cursor.mScaleX=0.65f;cursor.mScaleY=0.65f;SandboxPlants::DrawCard(&cursor,0,0,power);
 }
 if(noticeTime>0)PvzpDrawString(&g,notice,400,585,FONT_BRIANNETOD12,Color(255,224,140),DS_ALIGN_CENTER);
}
Save Capture(Board* b){Save out;out.power=power;out.cooldown=cooldown;
 for(auto* p:b->mPlants)if(!p->mDead&&SandboxPlants::IsCustom(p))out.plants.push_back({b->mPlants.DataArrayGetID(p),SandboxPlants::SavePower(p)});
 for(auto* shot:b->mProjectiles)if(!shot->mDead&&SandboxPlants::SaveShot(shot)!=100)out.shots.push_back({b->mProjectiles.DataArrayGetID(shot),SandboxPlants::SaveShot(shot)});return out;
}
void Load(const Save& save){pending=save;}
void LoadShots(const std::vector<SavedShot>& shots){pending.shots=shots;}
void Restore(Board* b){
 power=SandboxMemeRules::IsPower(pending.power)?pending.power:180;cooldown=std::clamp(pending.cooldown,0,MemeAdventureRules::Cooldown);selected=choosing=false;
 if(b->mApp->IsAdventureMode())for(const auto& saved:pending.plants)if(auto* p=b->mPlants.DataArrayTryToGet(saved.key))SandboxPlants::RestorePower(p,saved.state);
 if(b->mApp->IsAdventureMode())for(const auto& saved:pending.shots)if(auto* p=b->mProjectiles.DataArrayTryToGet(saved.key))SandboxPlants::RestoreShot(p,saved.percent);
 pending={};
}
}
// Read-only diagnostics for browser QA; no profile mutation or level unlock API.
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_adventure_power_data(int index,int field){
 auto* b=gLawnApp?gLawnApp->mBoard:nullptr;if(!b||gSandboxEnabled)return -1;
 const auto save=MemeAdventure::Capture(b);
 if(index==-1)return field==0?save.power:field==1?save.cooldown:field==2?b->mSunMoney:field==3?b->mLevel:field==4?int(save.plants.size()):field==5?int(MemeAdventure::Visible(b)):field==6?int(b->mPaused):-1;
 if(index>=0&&field>=10&&field<=11){int n=0;for(auto* c:b->mCoins)if(!c->mDead&&c->IsSun()&&!c->mIsBeingCollected){if(n++==index)return int(field==10?c->mPosX+30:c->mPosY+30);}return -1;}
 if(index<0||field<0||field>5)return -1;
 int n=0;for(auto* p:b->mPlants)if(!p->mDead){if(n++!=index)continue;
  return field==0?SandboxPlants::Type(p):field==1?p->mPlantCol:field==2?p->mRow:field==3?p->mPlantHealth:SandboxPlants::HeatData(p,field-4);
 }return -1;
}
