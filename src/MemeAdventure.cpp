// Adventure replacements retain native seed IDs, tutorial gates and save layout.
#include "MemeAdventure.h"
#include "MemeAdventureRules.h"
#include "Sandbox.h"
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
#include "graphics/Font.h"
#include "misc/SexyMatrix.h"
#include <map>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif
namespace MemeAdventure {
namespace {
Save pending;
bool fontsReady=false;
bool Running(Board* b){return Visible(b)&&!b->mPaused&&!b->mTimeStopCounter&&!b->mLevelComplete&&!b->mLevelAwardSpawned&&b->mApp->GetDialogCount()==0;}
}
bool RosterEnabled(){
 return gLawnApp&&!gSandboxEnabled&&gLawnApp->IsAdventureMode()&&!gLawnApp->IsWallnutBowlingLevel()&&!gLawnApp->IsScaryPotterLevel()&&!gLawnApp->IsWhackAZombieLevel();
}
const MemeCharacters::Definition* Replacement(int seed,int imitater){
 if(!RosterEnabled())return nullptr;
 return MemeCharacters::ForBase(seed==48?imitater:seed);
}
void Reset(){pending={};fontsReady=false;}
bool Visible(Board* b){return b&&RosterEnabled()&&b->mApp->mGameScene==SCENE_PLAYING;}
bool Cancel(){return false;} // Selection belongs to the native seed bank again.
void Tick(Board* b){if(Running(b))SandboxPlants::Tick(b);}
bool MouseDown(Board* b,int x,int y,int clicks){
 if(!Running(b)||clicks<0||b->mCursorObject->mCursorType!=CURSOR_TYPE_NORMAL)return false;
 HitResult hit;b->MouseHitTest(x,y,&hit);if(hit.mObjectType==OBJECT_TYPE_COIN)return false;
 return MemeCharacters::Click(b,x,y);
}
void OnPlanted(Plant* p){
 if(!p||p->mDead||MemeCharacters::Is(p))return;
 // Imitaters acquire the new identity when their normal morph creates the plant.
 if(const auto* d=Replacement(int(p->mSeedType)))MemeCharacters::Assign(p,d->id);
}
void Draw(Board* b,Sexy::Graphics*){
 if(Visible(b)&&!fontsReady){SandboxRepairFonts();fontsReady=true;}
 // No second tray or floating menu: native cards now own every planting action.
}
std::string_view Translate(std::string_view key,std::string_view original){
 if(gSandboxEnabled||RosterEnabled()){
  if(key=="NEWSPAPER_ZOMBIE")return "读手机僵尸";
  if(key=="NEWSPAPER_ZOMBIE_DESCRIPTION")return "边走边刷手机。手机碎了就红温冲锋，四秒后掏出备用机继续刷。身体不会回血。";
 }
 if(!RosterEnabled())return original;
 for(const auto& d:MemeCharacters::Definitions){const std::string_view stem=d.key;
  if(key==stem)return d.name;
  if(key.starts_with(stem)){const auto suffix=key.substr(stem.size());if(suffix=="_TOOLTIP")return d.hint;if(suffix=="_DESCRIPTION")return d.description;}
 }
 // Tutorial/warning text follows the new card names. Do not rewrite unrelated
 // terms such as twin sunflower, tall-nut, trophies or the bowling mini-game.
 if(!key.starts_with("ADVICE_")&&!key.starts_with("SEED_CHOOSER_")&&!key.starts_with("TUTORIAL_"))return original;
 static std::map<std::string,std::string,std::less<>> cache;
 if(auto found=cache.find(key);found!=cache.end())return found->second;
 std::string text(original);
 for(const auto& pair:{std::pair{"豌豆射手","红温豌豆"},std::pair{"向日葵","已读不回花"},std::pair{"小喷菇","显眼包蘑菇"},std::pair{"坚果墙","顶顶坚果"}}){
  size_t pos=0;const std::string_view from=pair.first,to=pair.second;
  while((pos=text.find(from,pos))!=std::string::npos){
   if(from=="向日葵"&&pos>=6&&text.compare(pos-6,6,"双子")==0){pos+=from.size();continue;}
   text.replace(pos,from.size(),to);pos+=to.size();
  }
 }
 return cache.emplace(std::string(key),std::move(text)).first->second;
}
Save Capture(Board* b){Save out;out.power=0;out.cooldown=0;
 for(auto* p:b->mPlants)if(!p->mDead&&MemeCharacters::Is(p))out.plants.push_back({b->mPlants.DataArrayGetID(p),SandboxPlants::SavePower(p)});
 for(auto* shot:b->mProjectiles)if(!shot->mDead&&SandboxPlants::SaveShot(shot)!=100)out.shots.push_back({b->mProjectiles.DataArrayGetID(shot),SandboxPlants::SaveShot(shot)});return out;
}
void Load(const Save& save){pending=save;}
void LoadShots(const std::vector<SavedShot>& shots){pending.shots=shots;}
void Restore(Board* b){
 if(b->mApp->IsAdventureMode())for(const auto& saved:pending.plants)if(auto* p=b->mPlants.DataArrayTryToGet(saved.key)){
  if(MemeCharacters::Is(saved.state[0]))SandboxPlants::RestorePower(p,saved.state);
  else if(SandboxMemeRules::IsResult(saved.state[0])){
   p->mLaunchRate=GetPlantDefinition(p->mSeedType).mLaunchRate;p->mLaunchCounter=std::max(100,p->mLaunchRate);p->mShootingCounter=0;
  }
 }
 // Also migrate ordinary plants in pre-mod saves, keeping HP and positions.
 if(RosterEnabled()){
  for(auto* p:b->mPlants)OnPlanted(p);
  if(b->mSeedBank)for(int i=0;i<b->mSeedBank->mNumPackets;++i){auto& card=b->mSeedBank->mSeedPackets[i];
   if(Replacement(int(card.mPacketType),int(card.mImitaterType))&&card.mRefreshing&&card.mRefreshTime>300){
    const int left=std::clamp(card.mRefreshTime-card.mRefreshCounter,0,300);card.mRefreshTime=300;card.mRefreshCounter=300-left;
   }
  }
 }
 if(b->mApp->IsAdventureMode())for(const auto& saved:pending.shots)if(auto* p=b->mProjectiles.DataArrayTryToGet(saved.key))SandboxPlants::RestoreShot(p,saved.percent);
 pending={};
}
}
// Read-only QA ABI; it cannot change player money, progression or gameplay speed.
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_adventure_power_data(int index,int field){
 auto* b=gLawnApp?gLawnApp->mBoard:nullptr;if(!b||gSandboxEnabled)return -1;const auto save=MemeAdventure::Capture(b);
 if(index==-1)return field==0?save.power:field==1?0:field==2?b->mSunMoney:field==3?b->mLevel:field==4?int(save.plants.size()):field==5?int(MemeAdventure::Visible(b)):field==6?int(b->mPaused):field==7?0:field==8?int(b->mTutorialState):field==9?int(b->mApp->mGameScene):-1;
 if(index>=0&&field>=10&&field<=13){int n=0;for(auto* c:b->mCoins)if(!c->mDead&&(field<12?c->IsSun():c->mType==COIN_FINAL_SEED_PACKET)&&!c->mIsBeingCollected){if(n++==index)return int(field%2==0?c->mPosX+30:c->mPosY+30);}return -1;}
 if(index<0||field<0||field>14)return -1;int n=0;for(auto* p:b->mPlants)if(!p->mDead){if(n++!=index)continue;return field==0?SandboxPlants::Type(p):field==1?p->mPlantCol:field==2?p->mRow:field==3?p->mPlantHealth:field==7?p->mLaunchCounter:field==8?int(p->mSeedType):field==9?int(p->mIsAsleep):field==14?MemeCharacters::Data(p,4):SandboxPlants::HeatData(p,field-4);}return -1;
}
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_adventure_seed_data(int index,int field){
 auto* b=gLawnApp?gLawnApp->mBoard:nullptr;if(!b||gSandboxEnabled||!b->mSeedBank||index<0||index>=b->mSeedBank->mNumPackets)return -1;
 const auto& card=b->mSeedBank->mSeedPackets[index];const auto* d=MemeAdventure::Replacement(int(card.mPacketType),int(card.mImitaterType));
 return field==0?(d?d->id:int(card.mPacketType)):field==1?Plant::GetCost(card.mPacketType,card.mImitaterType):field==2?Plant::GetRefreshTime(card.mPacketType,card.mImitaterType):field==3?(card.mRefreshing?card.mRefreshTime-card.mRefreshCounter:0):field==4?int(card.mActive):field==5?b->mSeedBank->mX+card.mX+card.mOffsetX:field==6?b->mSeedBank->mY+card.mY:-1;
}
