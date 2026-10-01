// Adventure replacements retain native seed IDs, tutorial gates and save layout.
#include "MemeAdventure.h"
#include "MemeAdventureRules.h"
#include "Sandbox.h"
#include "SandboxZombies.h"
#include "SandboxFonts.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Projectile.h"
#include "Lawn/Zombie.h"
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
void Tick(Board* b){if(Running(b)){SandboxPlants::Tick(b);SandboxZombies::Tick(b);}}
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
void OnZombieSpawned(Zombie* z){
 // Introduce originals gradually; previews, specials and tutorials stay native.
 if(!RosterEnabled()||!z||!z->mBoard||!z->IsOnBoard())return;
 const int coneVariant=SandboxZombies::ConeVariantWave(z->mBoard->mLevel,int(z->mZombieType),z->mFromWave,z->mBoard->IsPoolSquare(0,z->mRow));
 if(coneVariant>=0){
  bool exists=false;for(auto* other:z->mBoard->mZombies)if(other!=z&&SandboxZombies::Type(other)==coneVariant&&other->mFromWave==z->mFromWave)exists=true;
  if(!exists){SandboxZombies::Assign(z,coneVariant);return;}
 }
 const int clever=SandboxZombies::CleverWave(z->mBoard->mLevel,int(z->mZombieType),z->mFromWave);
 if(clever>=0){
  bool exists=false;for(auto* other:z->mBoard->mZombies)if(other!=z&&SandboxZombies::Type(other)==clever&&other->mFromWave==z->mFromWave)exists=true;
  if(!exists){SandboxZombies::Assign(z,clever);return;}
 }
 if(SandboxZombies::ConeWrapWave(z->mBoard->mLevel,int(z->mZombieType),z->mFromWave)){
  bool exists=false;for(auto* other:z->mBoard->mZombies)if(other!=z&&SandboxZombies::IsConeWrap(other)&&other->mFromWave==z->mFromWave)exists=true;
  if(!exists){SandboxZombies::Assign(z,SandboxZombies::ConeWrap);return;}
 }
 if(SandboxZombies::RunnerWave(z->mBoard->mLevel,int(z->mZombieType),z->mFromWave)){
  bool exists=false;for(auto* other:z->mBoard->mZombies)if(other!=z&&SandboxZombies::IsRunner(other)&&other->mFromWave==z->mFromWave)exists=true;
  if(!exists){SandboxZombies::Assign(z,SandboxZombies::Runner);return;}
 }
 if(SandboxZombies::LouisWave(z->mBoard->mLevel,int(z->mZombieType),z->mFromWave))SandboxZombies::Assign(z,SandboxZombies::Louis);
}
void Draw(Board* b,Sexy::Graphics*){
 if(Visible(b)&&!fontsReady){SandboxRepairFonts();fontsReady=true;}
 // No second tray or floating menu: native cards now own every planting action.
}
int PickWaveZombie(Board* b,int base,int row,int wave){
 // Select before native initialization so the new unit gets a real imp rig.
 // A giant's throw requests base 24 directly, so it can never enter here.
 if(!RosterEnabled()||!b||!SandboxZombies::GiantImpWave(b->mLevel,base,wave)||b->IsPoolSquare(0,row))return base;
 for(auto* z:b->mZombies)if(SandboxZombies::IsGiantImp(z)&&z->mFromWave==wave)return base;
 return SandboxZombies::GiantImp;
}
std::string_view Translate(std::string_view key,std::string_view original){
 const auto* tucking=MemeCharacters::Find(MemeCharacters::TuckingSunflower);
 if(key=="AWKWARD_SUNFLOWER")return tucking->name; // Legacy ID 53, never a second selectable card.
 if(key=="AWKWARD_SUNFLOWER_TOOLTIP")return tucking->hint;
 if(key=="AWKWARD_SUNFLOWER_DESCRIPTION")return tucking->description;
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
 for(const auto& pair:{std::pair{"豌豆射手","红温豌豆"},std::pair{"坚果墙","反咬坚果"},std::pair{"小喷菇","真·小喷菇"}}){
  size_t pos=0;const std::string_view from=pair.first,to=pair.second;
  while((pos=text.find(from,pos))!=std::string::npos){
   text.replace(pos,from.size(),to);pos+=to.size();
  }
 }
 return cache.emplace(std::string(key),std::move(text)).first->second;
}
Save Capture(Board* b){Save out;out.power=0;out.cooldown=RosterSaveVersion;
 for(auto* p:b->mPlants)if(!p->mDead&&MemeCharacters::Is(p))out.plants.push_back({b->mPlants.DataArrayGetID(p),SandboxPlants::SavePower(p)});
 for(auto* z:b->mZombies)if(!z->mDead&&SandboxZombies::Find(SandboxZombies::Type(z)))out.zombies.push_back({b->mZombies.DataArrayGetID(z),SandboxZombies::Type(z)});
 for(auto* shot:b->mProjectiles)if(!shot->mDead&&SandboxPlants::SaveShot(shot)!=100)out.shots.push_back({b->mProjectiles.DataArrayGetID(shot),SandboxPlants::SaveShot(shot)});return out;
}
void Load(const Save& save){pending.power=save.power;pending.cooldown=save.cooldown;pending.plants=save.plants;}
void LoadShots(const std::vector<SavedShot>& shots){pending.shots=shots;}
void LoadZombies(const std::vector<SavedZombie>& zombies){pending.zombies=zombies;}
void Restore(Board* b){
 if(b->mApp->IsAdventureMode())for(const auto& saved:pending.plants)if(auto* p=b->mPlants.DataArrayTryToGet(saved.key)){
  if(MemeCharacters::Is(saved.state[0]))SandboxPlants::RestorePower(p,saved.state);
  else SandboxPlants::RestoreRetired(p,saved.state);
 }
 // Also migrate ordinary plants in pre-mod saves, keeping HP and positions.
 if(RosterEnabled()){
  for(auto* p:b->mPlants){
   if(LegacyShooterSlot(pending.cooldown)&&int(p->mSeedType)==52&&!MemeCharacters::Is(p))SandboxPlants::RestoreRetired(p,{504,0,0,0,0,0,0,0,0,1});
   if(LegacySunflowerSlot(pending.cooldown)){
    if(int(p->mSeedType)==53&&!MemeCharacters::Is(p))p->mSeedType=SEED_SUNFLOWER;
    if(int(p->mImitaterType)==53)p->mImitaterType=SEED_SUNFLOWER;
   }
   OnPlanted(p);
  }
  if(b->mSeedBank)for(int i=0;i<b->mSeedBank->mNumPackets;++i){auto& card=b->mSeedBank->mSeedPackets[i];
   if(LegacySunflowerSlot(pending.cooldown)){
    if(int(card.mPacketType)==53)card.mPacketType=SEED_SUNFLOWER;
    if(int(card.mImitaterType)==53)card.mImitaterType=SEED_SUNFLOWER;
   }
   if(LegacyShooterSlot(pending.cooldown)){
    if(int(card.mPacketType)==52)card.mPacketType=SEED_PEASHOOTER;
    if(int(card.mImitaterType)==52)card.mImitaterType=SEED_PEASHOOTER;
   }
   if(const auto* replacement=Replacement(int(card.mPacketType),int(card.mImitaterType));replacement&&card.mRefreshing){
    const int cooldown=MemeCharacters::PlantingCooldown(replacement->id);
    if(card.mRefreshTime<cooldown){
     // Old three-second nut cards keep time already elapsed, but use 12s now.
     card.mRefreshCounter=std::clamp(card.mRefreshCounter,0,cooldown);card.mRefreshTime=cooldown;
    }else if(card.mRefreshTime>cooldown){
     const int left=std::clamp(card.mRefreshTime-card.mRefreshCounter,0,cooldown);card.mRefreshTime=cooldown;card.mRefreshCounter=cooldown-left;
    }
   }
  }
 }
 if(b->mApp->IsAdventureMode())for(const auto& saved:pending.shots)if(auto* p=b->mProjectiles.DataArrayTryToGet(saved.key))SandboxPlants::RestoreShot(p,saved.percent);
 if(b->mApp->IsAdventureMode())SandboxZombies::RestoreNative(b);
 if(b->mApp->IsAdventureMode())for(const auto& saved:pending.zombies)if(auto* z=b->mZombies.DataArrayTryToGet(saved.key))SandboxZombies::Restore(z,saved.type);
 pending={};
}
}
// Read-only QA ABI; it cannot change player money, progression or gameplay speed.
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_adventure_power_data(int index,int field){
 auto* b=gLawnApp?gLawnApp->mBoard:nullptr;if(!b||gSandboxEnabled)return -1;const auto save=MemeAdventure::Capture(b);
 if(index==-1)return field==0?save.power:field==1?0:field==2?b->mSunMoney:field==3?b->mLevel:field==4?int(save.plants.size()):field==5?int(MemeAdventure::Visible(b)):field==6?int(b->mPaused):field==7?0:field==8?int(b->mTutorialState):field==9?int(b->mApp->mGameScene):-1;
 if(index>=0&&field>=10&&field<=13){int n=0;for(auto* c:b->mCoins)if(!c->mDead&&(field<12?c->IsSun():c->mType==COIN_FINAL_SEED_PACKET)&&!c->mIsBeingCollected){if(n++==index)return int(field%2==0?c->mPosX+30:c->mPosY+30);}return -1;}
 // Read-only live-lane diagnostics let UI tests plant in an occupied lane;
 // they must not inject enemies or rely on random adventure spawn rows.
 if(index>=0&&(field==16||field==17)){int n=0;for(auto* z:b->mZombies)if(z->IsOnBoard()&&!z->IsDeadOrDying()&&!z->mMindControlled&&z->mPosX>350){if(n++==index)return field==16?z->mRow:int(z->mPosX);}return -1;}
 if(index<0||field<0||field>15)return -1;int n=0;for(auto* p:b->mPlants)if(!p->mDead){if(n++!=index)continue;return field==0?SandboxPlants::Type(p):field==1?p->mPlantCol:field==2?p->mRow:field==3?p->mPlantHealth:field==7?p->mLaunchCounter:field==8?int(p->mSeedType):field==9?int(p->mIsAsleep):field==14?MemeCharacters::Data(p,4):field==15?MemeCharacters::Data(p,5):SandboxPlants::HeatData(p,field-4);}return -1;
}
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_adventure_seed_data(int index,int field){
 auto* b=gLawnApp?gLawnApp->mBoard:nullptr;if(!b||gSandboxEnabled||!b->mSeedBank||index<0||index>=b->mSeedBank->mNumPackets)return -1;
 const auto& card=b->mSeedBank->mSeedPackets[index];const auto* d=MemeAdventure::Replacement(int(card.mPacketType),int(card.mImitaterType));
 return field==0?(d?d->id:int(card.mPacketType)):field==1?Plant::GetCost(card.mPacketType,card.mImitaterType):field==2?Plant::GetRefreshTime(card.mPacketType,card.mImitaterType):field==3?(card.mRefreshing?card.mRefreshTime-card.mRefreshCounter:0):field==4?int(card.mActive):field==5?b->mSeedBank->mX+card.mX+card.mOffsetX:field==6?b->mSeedBank->mY+card.mY:-1;
}
