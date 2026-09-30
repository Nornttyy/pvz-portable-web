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
struct Coin {CoinType mType;int GetSunValue();};
struct Board {
 struct {int mSize=0,mMaxSize=1000;} mCoins;
 struct Challenge {ChallengeState mChallengeState=STATECHALLENGE_LAST_STAND_ONSLAUGHT;} challenge;
 Challenge* mChallenge=&challenge;int mCurrentWave=1,mNumWaves=10;bool award=false;
 std::vector<Coin> coins;std::vector<CoinMotion> motions;
 bool HasLevelAwardDropped(){return award;}
 void AddCoin(int,int,CoinType type,CoinMotion motion){coins.push_back({type});motions.push_back(motion);++mCoins.mSize;}
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
int RandRangeInt(int low,int high){assert(low<=high);return high;}
float RandRangeFloat(float low,float high){assert(low<=high);return high;}
int PvzpAnimateCurve(int,int,int,int,int,PvzpCurves){return 0;}
namespace Sexy {int Rand(int){return 50;}}
// The runner appends the actual production/growth/value implementations.
int main(){
 for(bool replacement:{false,true}){
  Plant p;p.meme=replacement;p.UpdateProductionPlant();assert(p.board.coins.size()==1);
  assert(p.board.coins[0].GetSunValue()==50&&p.board.motions[0]==COIN_MOTION_FROM_PLANT);
  assert(p.mLaunchCounter==2500);p.UpdateProductionPlant();assert(p.board.coins.size()==1&&p.mLaunchCounter==2499);
 }
 for(auto state:{STATE_SUNSHROOM_SMALL,STATE_SUNSHROOM_BIG}){
  Plant p;p.mSeedType=SEED_SUNSHROOM;p.mState=state;p.mStateCountdown=100;p.UpdateSunShroom();
  assert(p.board.coins.size()==1&&p.board.coins[0].GetSunValue()==(state==STATE_SUNSHROOM_SMALL?15:50));
 }
 {Plant p;p.mSeedType=SEED_SUNSHROOM;p.mState=STATE_SUNSHROOM_GROWING;p.UpdateSunShroom();assert(p.board.coins.empty()&&p.mLaunchCounter==1);
  p.app.animation.mLoopCount=1;p.UpdateSunShroom();assert(p.mState==STATE_SUNSHROOM_BIG&&p.board.coins.empty());
  p.UpdateSunShroom();assert(p.board.coins.size()==1&&p.board.coins[0].GetSunValue()==50);}
 {Plant p;p.mSeedType=SEED_TWINSUNFLOWER;p.UpdateProductionPlant();assert(p.board.coins.size()==2);for(auto c:p.board.coins)assert(c.GetSunValue()==25);}
 {Plant p;p.app.mGameMode=GAMEMODE_CHALLENGE_BIG_TIME;p.UpdateProductionPlant();assert(p.board.coins.size()==2);for(auto c:p.board.coins)assert(c.GetSunValue()==50);}
 for(int guard=0;guard<5;++guard){Plant p;switch(guard){case 0:p.meme=true;p.producing=false;break;case 1:p.inPlay=false;break;case 2:p.app.izombie=true;break;case 3:p.board.award=true;break;case 4:p.board.mCoins.mSize=992;break;}p.UpdateProductionPlant();assert(p.board.coins.empty());}
 {Coin sky{COIN_SUN};assert(sky.GetSunValue()==25);}
 std::cout<<"Native sun production: sunflower 50, mature shroom 50, young 15; timing, guards and sky unchanged\n";
}
