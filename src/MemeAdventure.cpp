// Fixed-character seed drawer. Native adventure progress and seed bank are intact.
#include "MemeAdventure.h"
#include "MemeAdventureRules.h"
#include "Sandbox.h"
#include "SandboxButton.h"
#include "SandboxFonts.h"
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
int power=500,cooldown=0,noticeTime=0;
bool choosing=false,selected=false,fontsReady=false;
std::string notice;
Save pending;
bool Running(Board* b){return Visible(b)&&!b->mPaused&&!b->mTimeStopCounter&&!b->mLevelComplete&&!b->mLevelAwardSpawned&&b->mApp->GetDialogCount()==0;}
void Say(const char* text){notice=text;noticeTime=140;}
bool Unlocked(Board* b,int id){const auto* d=MemeCharacters::Find(id);return d&&(b->mApp->HasFinishedAdventure()||b->mLevel>=d->unlock);}
}
void Reset(){power=500;cooldown=0;noticeTime=0;choosing=selected=fontsReady=false;notice.clear();pending={};}
int UIState(){return choosing?1:selected?2:0;}
bool Visible(Board* b){return b&&!gSandboxEnabled&&b->mApp->IsAdventureMode()&&b->mApp->mGameScene==SCENE_PLAYING&&!b->HasConveyorBeltSeedBank()&&!b->mApp->IsChallengeWithoutSeedBank()&&!b->mApp->IsWallnutBowlingLevel()&&!b->mApp->IsScaryPotterLevel();}
bool Cancel(){const bool used=selected||choosing;selected=choosing=false;return used;}
void Tick(Board* b){if(!Running(b)){Cancel();return;}if(cooldown)--cooldown;if(noticeTime)--noticeTime;SandboxPlants::Tick(b);}
bool MouseDown(Board* b,int x,int y,int clicks){
 using namespace MemeAdventureRules;
 if(!Running(b)){Cancel();return false;}
 if(clicks<0)return Cancel();
 if(Slot.Contains(x,y)){b->RefreshSeedPacketFromCursor();b->ClearCursor();selected=false;choosing=!choosing;b->mApp->PlaySample(Sexy::SOUND_SEEDLIFT);return true;}
 if(choosing){
  for(int i=0;i<4;++i)if(CharacterCard(i).Contains(x,y)){
   const auto& d=MemeCharacters::Definitions[i];
   if(!Unlocked(b,d.id)){notice=std::format("{} 解锁",b->mApp->GetStageString(d.unlock));noticeTime=140;return true;}
   if(cooldown){Say("冷却中");return true;}
   if(!b->CanTakeSunMoney(d.cost)){Say("阳光不足");b->mOutOfMoneyCounter=70;return true;}
   power=d.id;selected=true;choosing=false;b->mApp->PlaySample(Sexy::SOUND_SEEDLIFT);return true;
  }
  choosing=false;
 }
 HitResult hit;b->MouseHitTest(x,y,&hit);if(hit.mObjectType==OBJECT_TYPE_COIN)return false;
 if(!selected)return b->mCursorObject->mCursorType==CURSOR_TYPE_NORMAL&&MemeCharacters::Click(b,x,y);
 if(y<82){selected=false;return false;}
 const auto* d=MemeCharacters::Find(power);if(!d)return true;
 const int col=b->PixelToGridX(x,y),row=b->PixelToGridY(x,y);
 if(col<0||col>=9||row<0||row>=(b->StageHasPool()?6:5)||b->CanPlantAt(col,row,static_cast<SeedType>(d->base))!=PLANTING_OK){Say("这里不能种植");return true;}
 if(cooldown||!Unlocked(b,power)||!b->CanTakeSunMoney(d->cost))return true;
 if(b->mPlants.mSize>=b->mPlants.mMaxSize-8){Say("场地已满");return true;}
 Plant::PreloadPlantResources(static_cast<SeedType>(d->base));
 auto* p=b->AddPlant(col,row,static_cast<SeedType>(d->base),SEED_NONE);if(!p)return true;
 b->TakeSunMoney(d->cost);SandboxPlants::Assign(p,power);cooldown=Cooldown;selected=false;noticeTime=0;
 b->mApp->PlaySample(Sexy::SOUND_PLANTGROW);b->MarkAllDirty();return true;
}
void OnPlanted(Plant*){} // Native cards no longer accept infusion.
void Draw(Board* b,Sexy::Graphics* graphics){
 if(!Visible(b)||b->mLevelComplete||b->mLevelAwardSpawned)return;
 if(!fontsReady){SandboxRepairFonts();fontsReady=true;}
 using namespace Sexy;using namespace MemeAdventureRules;
 Graphics g(*graphics);const auto* wm=b->mApp->mWidgetManager.get();const int mx=wm->mLastMouseX-b->mX,my=wm->mLastMouseY-b->mY;
 SandboxDrawButton(&g,Slot,cooldown?std::format("新品 {}s",(cooldown+99)/100):"新品",false,selected||choosing||Slot.Contains(mx,my));
 if(choosing){
  // Crop away the seed bank's sun counter before adapting its wooden tray.
  g.DrawImage(IMAGE_SEEDBANK,Rect(548,82,244,131),Rect(85,0,IMAGE_SEEDBANK->mWidth-85,IMAGE_SEEDBANK->mHeight));
  const char* hint="选卡后直接种植";
  for(int i=0;i<4;++i){const auto box=CharacterCard(i);const auto& d=MemeCharacters::Definitions[i];
   SandboxPlants::DrawCard(&g,box.x+2,box.y+2,d.id);
   const bool locked=!Unlocked(b,d.id);
   if(locked||cooldown){g.SetColor(Color(0,0,0,135));g.FillRect(box.x+2,box.y+2,50,70);}
   PvzpDrawString(&g,locked?b->mApp->GetStageString(d.unlock):std::string(d.shortName),box.x+27,box.y+89,FONT_BRIANNETOD12,Color(244,216,120),DS_ALIGN_CENTER);
   if(box.Contains(mx,my))hint=d.hint;
  }
  PvzpDrawString(&g,hint,670,204,FONT_BRIANNETOD12,Color(244,216,120),DS_ALIGN_CENTER);
 }
 if(selected){const auto* d=MemeCharacters::Find(power);const int col=b->PixelToGridX(mx,my),row=b->PixelToGridY(mx,my);
  if(d&&col>=0&&col<9&&row>=0&&row<(b->StageHasPool()?6:5)){
   const bool valid=b->CanPlantAt(col,row,static_cast<SeedType>(d->base))==PLANTING_OK;
   g.SetColor(valid?Color(100,230,70,85):Color(220,60,40,85));g.FillRect(b->GridToPixelX(col,row),b->GridToPixelY(col,row),80,b->StageHasPool()?85:100);
  }
  Graphics cursor(g);cursor.Translate(std::clamp(mx+12,0,748),std::clamp(my-65,0,528));SandboxPlants::DrawCard(&cursor,0,0,power);
 }
 if(noticeTime)PvzpDrawString(&g,notice,400,585,FONT_BRIANNETOD12,Color(255,224,140),DS_ALIGN_CENTER);
}
Save Capture(Board* b){Save out;out.power=power;out.cooldown=cooldown;
 for(auto* p:b->mPlants)if(!p->mDead&&MemeCharacters::Is(p))out.plants.push_back({b->mPlants.DataArrayGetID(p),SandboxPlants::SavePower(p)});
 for(auto* shot:b->mProjectiles)if(!shot->mDead&&SandboxPlants::SaveShot(shot)!=100)out.shots.push_back({b->mProjectiles.DataArrayGetID(shot),SandboxPlants::SaveShot(shot)});return out;
}
void Load(const Save& save){pending=save;}
void LoadShots(const std::vector<SavedShot>& shots){pending.shots=shots;}
void Restore(Board* b){
 power=MemeCharacters::Is(pending.power)?pending.power:500;cooldown=std::clamp(pending.cooldown,0,MemeAdventureRules::Cooldown);selected=choosing=false;
 if(b->mApp->IsAdventureMode())for(const auto& saved:pending.plants)if(auto* p=b->mPlants.DataArrayTryToGet(saved.key)){
  if(MemeCharacters::Is(saved.state[0]))SandboxPlants::RestorePower(p,saved.state);
  else if(SandboxMemeRules::IsResult(saved.state[0])){
   // Retire old infusion safely: restore native clocks, not HP or level progress.
   p->mLaunchRate=GetPlantDefinition(p->mSeedType).mLaunchRate;p->mLaunchCounter=std::max(100,p->mLaunchRate);p->mShootingCounter=0;
  }
 }
 if(b->mApp->IsAdventureMode())for(const auto& saved:pending.shots)if(auto* p=b->mProjectiles.DataArrayTryToGet(saved.key))SandboxPlants::RestoreShot(p,saved.percent);
 pending={};
}
}
// Keep the read-only QA ABI; no profile mutation or level-unlock endpoint.
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_adventure_power_data(int index,int field){
 auto* b=gLawnApp?gLawnApp->mBoard:nullptr;if(!b||gSandboxEnabled)return -1;const auto save=MemeAdventure::Capture(b);
 if(index==-1)return field==0?save.power:field==1?save.cooldown:field==2?b->mSunMoney:field==3?b->mLevel:field==4?int(save.plants.size()):field==5?int(MemeAdventure::Visible(b)):field==6?int(b->mPaused):field==7?MemeAdventure::UIState():-1;
 if(index>=0&&field>=10&&field<=11){int n=0;for(auto* c:b->mCoins)if(!c->mDead&&c->IsSun()&&!c->mIsBeingCollected){if(n++==index)return int(field==10?c->mPosX+30:c->mPosY+30);}return -1;}
 if(index<0||field<0||field>6)return -1;int n=0;for(auto* p:b->mPlants)if(!p->mDead){if(n++!=index)continue;return field==0?SandboxPlants::Type(p):field==1?p->mPlantCol:field==2?p->mRow:field==3?p->mPlantHealth:SandboxPlants::HeatData(p,field-4);}return -1;
}
