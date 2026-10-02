#include <cassert>
#include <algorithm>
#include <cmath>
#include <iostream>
enum class CoinType{COIN_SILVER,COIN_GOLD,COIN_DIAMOND,COIN_SUN,COIN_SMALLSUN,COIN_LARGESUN,COIN_FINAL_SEED_PACKET,COIN_TROPHY,COIN_AWARD_MONEY_BAG,COIN_AWARD_BAG_DIAMOND,COIN_USABLE_SEED_PACKET,COIN_PRESENT_PLANT,COIN_AWARD_PRESENT,COIN_PRESENT_MINIGAMES,COIN_PRESENT_PUZZLE_MODE,COIN_PRESENT_SURVIVAL_MODE};
enum class CoinMotion{COIN_MOTION_COIN,COIN_MOTION_FROM_PLANT,COIN_MOTION_FROM_PRESENT};
enum class GameScenes{SCENE_PLAYING,SCENE_AWARD,SCENE_LEVEL_INTRO,SCENE_ZOMBIES_WON};
enum class GameMode{NORMAL,GAMEMODE_CHALLENGE_ZEN_GARDEN};
enum class CrazyDaveState{CRAZY_DAVE_OFF,ON};
enum class Dialogs{DIALOG_STORE};
enum class AttachmentID{ATTACHMENTID_NULL};
enum class AdviceType{ADVICE_UNLOCKED_MODE};
enum class MessageStyle{MESSAGE_STYLE_HINT_TALL_UNLOCKMESSAGE};
enum class PvzpCurves{CURVE_EASE_IN_OUT,CURVE_EASE_OUT};
struct Color{Color(int=0,int=0,int=0,int=0){}};
float PvzpAnimateCurveFloat(int,int,int,float,float,PvzpCurves){return 1;}
void AttachmentUpdateAndMove(AttachmentID,float,float){}void AttachmentOverrideColor(AttachmentID,Color){}void AttachmentOverrideScale(AttachmentID,float){}
struct Profile{int mCoins=100;void AddCoins(int n){mCoins+=n;}};
struct App{Profile profile;Profile* mPlayerInfo=&profile;GameScenes mGameScene=GameScenes::SCENE_PLAYING;GameMode mGameMode=GameMode::NORMAL;CrazyDaveState mCrazyDaveState=CrazyDaveState::CRAZY_DAVE_OFF;int dialogs=0;int GetDialogCount(){return dialogs;}bool GetDialog(int){return false;}};
struct Cutscene{bool ShouldRunUpsellBoard(){return false;}};
struct Advice{bool IsBeingDisplayed(){return false;}};
struct Displayed{bool operator[](AdviceType){return true;}};
struct Board{bool mPaused=false;int mCoinsCollected=0,mLevelCoinsCollected=0,mDiamondsCollected=0,sun=0;Cutscene cutscene;Cutscene* mCutScene=&cutscene;Advice advice;Advice* mAdvice=&advice;Displayed mHelpDisplayed;AdviceType mHelpIndex=AdviceType::ADVICE_UNLOCKED_MODE;void AddSunMoney(int n){sun+=n;}void DisplayAdvice(const char*,MessageStyle,AdviceType){}};
constexpr int PennyPincher=0;int achievements=0;
namespace ReportAchievement{void GiveAchievement(App*,int,bool){achievements++;}}
struct Coin{
 App* mApp;Board* mBoard;CoinType mType;CoinMotion mCoinMotion=CoinMotion::COIN_MOTION_COIN;
 bool mDead=false,mIsBeingCollected=false,mHitGround=false;int mCoinAge=0,mFadeCount=0,mDisappearCounter=0,mWidth=60,mHeight=60,sounds=0,collections=0,fallUpdates=0;
 float mPosX=320,mPosY=250,mScale=1,mCollectionDistance=0,mCollectX=0,mCollectY=0;AttachmentID mAttachmentID=AttachmentID::ATTACHMENTID_NULL;
 Coin(App& app,Board& board,CoinType type):mApp(&app),mBoard(&board),mType(type){}
 void TryAutoCollectCoin();void Update();void UpdateCollected();void ScoreCoin();bool IsSun();bool IsMoney();static bool IsMoney(CoinType);static int GetCoinValue(CoinType);int GetSunValue();float GetSunScale();
 void Collect(){mIsBeingCollected=true;mFadeCount=0;mCollectX=mPosX;mCollectY=mPosY;collections++;}
 void PlayCollectSound(){sounds++;}void Die(){mDead=true;}void UpdateFall(){fallUpdates++;}void UpdateFade(){if(!--mFadeCount)Die();}void StartFade(){mFadeCount=15;}
 bool IsLevelAward(){return mType==CoinType::COIN_TROPHY||mType==CoinType::COIN_FINAL_SEED_PACKET;}
 bool IsPresentWithAdvice(){return false;}Color GetColor(){return {};}
};
void finish(Coin& c){for(int i=0;i<500&&!c.mDead;i++)c.Update();assert(c.mDead);}
int main(){
 for(auto type:{CoinType::COIN_SILVER,CoinType::COIN_GOLD}){
  App app;Board board;Coin c(app,board,type);for(int i=0;i<59;i++)c.Update();assert(!c.mIsBeingCollected&&c.sounds==0&&c.fallUpdates==59);
  c.Update();assert(c.mIsBeingCollected&&c.sounds==1&&c.collections==1&&app.profile.mCoins==100);
  finish(c);assert(app.profile.mCoins==100+Coin::GetCoinValue(type)&&board.mCoinsCollected==Coin::GetCoinValue(type)&&board.mLevelCoinsCollected==1);
  for(int i=0;i<20;i++)c.TryAutoCollectCoin();assert(c.sounds==1&&c.collections==1);
 }
 for(auto type:{CoinType::COIN_DIAMOND,CoinType::COIN_SUN,CoinType::COIN_SMALLSUN,CoinType::COIN_LARGESUN,CoinType::COIN_FINAL_SEED_PACKET,CoinType::COIN_TROPHY,CoinType::COIN_AWARD_MONEY_BAG,CoinType::COIN_AWARD_BAG_DIAMOND,CoinType::COIN_USABLE_SEED_PACKET,CoinType::COIN_PRESENT_PLANT,CoinType::COIN_AWARD_PRESENT}){
  App app;Board board;Coin c(app,board,type);c.mCoinAge=1000;c.TryAutoCollectCoin();assert(!c.mIsBeingCollected&&c.sounds==0&&app.profile.mCoins==100);
 }
 for(int guard=0;guard<9;guard++){
  App app;Board board;Coin c(app,board,CoinType::COIN_GOLD);c.mCoinAge=90;
  switch(guard){case 0:c.mDead=true;break;case 1:c.mIsBeingCollected=true;break;case 2:c.mBoard=nullptr;break;case 3:board.mPaused=true;break;case 4:app.dialogs=1;break;case 5:app.mGameScene=GameScenes::SCENE_LEVEL_INTRO;break;case 6:app.mGameScene=GameScenes::SCENE_AWARD;break;case 7:app.mPlayerInfo=nullptr;break;case 8:c.mCoinMotion=CoinMotion::COIN_MOTION_FROM_PRESENT;break;}
  c.TryAutoCollectCoin();assert(c.collections==0&&c.sounds==0);
 }
 {App app;Board board;Coin c(app,board,CoinType::COIN_GOLD);c.mCoinAge=70;board.mPaused=true;c.TryAutoCollectCoin();assert(!c.mIsBeingCollected);board.mPaused=false;c.TryAutoCollectCoin();finish(c);assert(app.profile.mCoins==105);}
 {App app;Board board;Coin c(app,board,CoinType::COIN_GOLD);c.mCoinAge=80;c.mFadeCount=3;c.Update();assert(c.mFadeCount==0&&c.collections==1);finish(c);assert(app.profile.mCoins==105);}
 {App app;Board board;Coin c(app,board,CoinType::COIN_SILVER);c.Collect();finish(c);assert(c.collections==1&&c.sounds==0&&app.profile.mCoins==101);}
 {App app;Board board;Coin c(app,board,CoinType::COIN_GOLD);c.mCoinAge=59;c.Update();Coin resumed=c;finish(resumed);assert(resumed.collections==1&&resumed.sounds==1&&app.profile.mCoins==105);}
 {App app;Board board;Coin c(app,board,CoinType::COIN_SILVER);c.mCoinAge=59;Coin resumed=c;finish(resumed);assert(resumed.collections==1&&app.profile.mCoins==101);}
 {App app;Board board;for(int i=0;i<30;i++){Coin c(app,board,CoinType::COIN_SILVER);finish(c);}assert(app.profile.mCoins==130&&board.mLevelCoinsCollected==30&&achievements==1);}
 std::cout<<"Auto coins: timing, native flight/credit, exclusions, pause, manual race, restored state and achievements passed\n";
}
