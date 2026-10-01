#include "ConstEnums.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
enum PlantState {STATE_NOTREADY,STATE_SUNSHROOM_SMALL,STATE_SUNSHROOM_GROWING,STATE_SUNSHROOM_BIG,STATE_MARIGOLD_ENDING};
enum FoleyType {FOLEY_SPAWN_SUN,FOLEY_PLANTGROW};
struct Reanimation {int mLoopCount=0;};
struct App {
 GameMode mGameMode=GAMEMODE_ADVENTURE;bool izombie=false;Reanimation animation;
 bool IsIZombieLevel(){return izombie;}void PlayFoley(FoleyType){}
 Reanimation* ReanimationGet(int){return &animation;}
};
struct Coin {CoinType mType;CoinMotion mCoinMotion=COIN_MOTION_FROM_SKY;int GetSunValue();float GetSunScale();};
struct Board {
 struct {int mSize=0,mMaxSize=1000;} mCoins;
 struct Challenge {ChallengeState mChallengeState=STATECHALLENGE_LAST_STAND_ONSLAUGHT;} challenge;
 Challenge* mChallenge=&challenge;int mCurrentWave=1,mNumWaves=10;bool award=false;
 std::vector<Coin> coins;std::vector<CoinMotion> motions;
 bool HasLevelAwardDropped(){return award;}
 void AddCoin(int,int,CoinType type,CoinMotion motion){coins.push_back({type,motion});motions.push_back(motion);++mCoins.mSize;}
};
struct Plant {
 App app;Board board;App* mApp=&app;Board* mBoard=&board;
 SeedType mSeedType=SEED_SUNFLOWER;PlantState mState=STATE_NOTREADY;
 int mLaunchCounter=1,mLaunchRate=2500,mEatenFlashCountdown=0,mStateCountdown=0,mX=0,mY=0,mBodyReanimID=0;
 bool inPlay=true,meme=false,producing=true;int copies=1;
 bool IsInPlay(){return inPlay;}void UpdateProductionPlant();void UpdateSunShroom();
 void PlayBodyReanim(const char*,ReanimLoopType,int,float){}
};
namespace MemeCharacters {bool Is(Plant* p){return p->meme;}bool Producing(Plant* p){return p->producing;}}
namespace SandboxPlants {int NativeProduction(Plant* p){return p->copies;}}
bool useLowRoll=false;
int RandRangeInt(int low,int high){assert(low<=high);return useLowRoll?low:high;}
float RandRangeFloat(float low,float high){assert(low<=high);return high;}
int PvzpAnimateCurve(int,int,int,int,int,PvzpCurves){return 0;}
namespace Sexy {int Rand(int){return 50;}}
// The runner appends the actual production/growth/value implementations.
int main(){
 {Plant p;p.mSeedType=SEED_SUNFLOWER;p.meme=true;p.UpdateProductionPlant();
  assert(p.board.coins.size()==1&&p.board.coins[0].GetSunValue()==25);
  p.UpdateProductionPlant();assert(p.mLaunchCounter==2498);
  for(int i=0;i<400;++i)p.UpdateProductionPlant();assert(p.mLaunchCounter==1698&&p.board.coins.size()==1);
  p.UpdateProductionPlant();assert(p.mLaunchCounter==1696);
  p.board.award=true;const int before=p.mLaunchCounter;p.UpdateProductionPlant();assert(p.mLaunchCounter==before);
 }
 {Plant p;p.mSeedType=SEED_SUNFLOWER;p.board.mCoins.mSize=992;p.UpdateProductionPlant();assert(p.board.coins.empty());}
 {Plant p;p.mSeedType=SEED_SMALL_NUT;p.meme=true;p.producing=false;p.UpdateProductionPlant();assert(p.board.coins.empty()&&p.mLaunchCounter==1);}
 for(bool replacement:{false,true}){
  Plant p;p.meme=replacement;p.UpdateProductionPlant();assert(p.board.coins.size()==1);
  assert(p.board.coins[0].GetSunValue()==25&&p.board.motions[0]==COIN_MOTION_FROM_PLANT);
  assert(p.board.coins[0].GetSunScale()==1.0f);
  assert(p.mLaunchCounter==2500);p.UpdateProductionPlant();assert(p.board.coins.size()==1&&p.mLaunchCounter==2498);
 }
 for(auto state:{STATE_SUNSHROOM_SMALL,STATE_SUNSHROOM_BIG}){
  Plant p;p.mSeedType=SEED_SUNSHROOM;p.mState=state;p.mStateCountdown=100;p.UpdateSunShroom();
  assert(p.board.coins.size()==1&&p.board.coins[0].GetSunValue()==(state==STATE_SUNSHROOM_SMALL?15:25));
  assert(p.board.coins[0].GetSunScale()==(state==STATE_SUNSHROOM_SMALL?0.5f:1.0f));
 }
 {Plant p;p.mSeedType=SEED_SUNSHROOM;p.mState=STATE_SUNSHROOM_GROWING;p.UpdateSunShroom();assert(p.board.coins.empty()&&p.mLaunchCounter==1);
  p.app.animation.mLoopCount=1;p.UpdateSunShroom();assert(p.mState==STATE_SUNSHROOM_BIG&&p.board.coins.empty());
  p.UpdateSunShroom();assert(p.board.coins.size()==1&&p.board.coins[0].GetSunValue()==25);}
 {Plant p;p.mSeedType=SEED_TWINSUNFLOWER;p.UpdateProductionPlant();assert(p.board.coins.size()==2);for(auto c:p.board.coins)assert(c.GetSunValue()==25);}
 {Plant p;p.app.mGameMode=GAMEMODE_CHALLENGE_BIG_TIME;p.UpdateProductionPlant();assert(p.board.coins.size()==2);for(auto c:p.board.coins)assert(c.GetSunValue()==25);}
 // Test the actual reset/decrement loop at both native random bounds. Only
 // sunflower (including 503) and mature shroom run at twice the old frequency.
 for(bool low:{false,true})for(int kind=0;kind<6;++kind){
  useLowRoll=low;Plant p;
  if(kind==1)p.meme=true;
  if(kind==2||kind==3){p.mSeedType=SEED_SUNSHROOM;p.mState=kind==2?STATE_SUNSHROOM_BIG:STATE_SUNSHROOM_SMALL;}
  if(kind==4)p.mSeedType=SEED_TWINSUNFLOWER;
  if(kind==5)p.mSeedType=SEED_MARIGOLD;
  p.UpdateProductionPlant();const auto initial=p.board.coins.size();
  const int ticks=(low?2350:2500)/(kind<3?2:1);
  for(int i=1;i<ticks;++i){p.UpdateProductionPlant();assert(p.board.coins.size()==initial);}
  p.UpdateProductionPlant();assert(p.board.coins.size()==initial*2);
 }
 useLowRoll=false;
 // Old saved countdowns need no migration, and odd counters still emit once.
 for(int remaining:{1,2,3,99,100,101,1201,2500}){
  Plant p;p.mLaunchCounter=remaining;
  for(int i=1;i<(remaining+1)/2;++i){p.UpdateProductionPlant();assert(p.board.coins.empty());}
  p.UpdateProductionPlant();assert(p.board.coins.size()==1&&p.mLaunchCounter==2500);
 }
 {Plant p;p.meme=true;p.producing=false;p.mLaunchCounter=1200;p.UpdateProductionPlant();assert(p.mLaunchCounter==1200);
  p.producing=true;p.UpdateProductionPlant();assert(p.mLaunchCounter==1198);}
 // Tucking freezes even a nearly-due sun; no spawn, flash or reset while
 // hidden. Repeated hide/recover transitions resume the exact native timer.
 for(int remaining:{1,2,99,100,101,821,2500}){
  Plant p;p.meme=true;p.producing=false;p.mLaunchCounter=remaining;
  for(int tick=0;tick<5000;++tick)p.UpdateProductionPlant();
  assert(p.mLaunchCounter==remaining&&p.board.coins.empty()&&p.mEatenFlashCountdown==0);
  for(int tick=1;tick<(remaining+1)/2;++tick){
   p.producing=true;p.UpdateProductionPlant();assert(p.board.coins.empty());
   const int before=p.mLaunchCounter;p.producing=false;for(int hidden=0;hidden<3;++hidden)p.UpdateProductionPlant();assert(p.mLaunchCounter==before);
  }
  p.producing=true;p.UpdateProductionPlant();assert(p.board.coins.size()==1&&p.board.coins[0].GetSunValue()==25&&p.mLaunchCounter==2500);
 }
 for(int guard=0;guard<5;++guard){Plant p;switch(guard){case 0:p.meme=true;p.producing=false;break;case 1:p.inPlay=false;break;case 2:p.app.izombie=true;break;case 3:p.board.award=true;break;case 4:p.board.mCoins.mSize=992;break;}p.UpdateProductionPlant();assert(p.board.coins.empty());}
 {Coin sky{COIN_SUN};assert(sky.GetSunValue()==25);}
 for(auto motion:{COIN_MOTION_FROM_SKY,COIN_MOTION_FROM_SKY_SLOW,COIN_MOTION_FROM_PLANT,COIN_MOTION_COIN,COIN_MOTION_FROM_PRESENT}){
  Coin sun{COIN_LARGESUN,motion};assert(sun.GetSunValue()==50);
  assert(sun.GetSunScale()==(motion==COIN_MOTION_FROM_PLANT?1.0f:2.0f));
 }
 std::cout<<"Native sun production: sunflower 25, mature shroom 25 at double frequency; young 15, twin, guards and sky unchanged\n";
}
