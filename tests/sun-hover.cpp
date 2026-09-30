#include <cassert>
#include <iostream>
#include <vector>
enum class GameScenes {SCENE_PLAYING,SCENE_LEVEL_INTRO,SCENE_ZOMBIES_WON};
enum class CoinType {COIN_SUN,COIN_SMALLSUN,COIN_LARGESUN,COIN_GOLD,COIN_PRESENT_PLANT,COIN_AWARD_PRESENT,COIN_USABLE_SEED_PACKET,COIN_FINAL_SEED_PACKET};
enum class CursorType {CURSOR_TYPE_NORMAL,CURSOR_TYPE_PLANT_FROM_BANK,CURSOR_TYPE_SHOVEL};
enum class GameObjectType {OBJECT_TYPE_NONE,OBJECT_TYPE_COIN};
enum class MessageStyle {MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY};
enum class AdviceType {ADVICE_CLICKED_ON_SUN};
struct HitResult {void* mObject=nullptr;GameObjectType mObjectType=GameObjectType::OBJECT_TYPE_NONE;};
struct App {
 GameScenes mGameScene=GameScenes::SCENE_PLAYING;int dialogs=0;bool whack=false;
 int GetDialogCount(){return dialogs;}bool IsWhackAZombieLevel(){return whack;}bool IsFirstTimeAdventureMode(){return true;}
};
struct Cursor {CursorType mCursorType=CursorType::CURSOR_TYPE_NORMAL;};
struct Coin;
struct Board {
 App app;App* mApp=&app;Cursor cursor;Cursor* mCursorObject=&cursor;
 bool mPaused=false,talking=false;int mTimeStopCounter=0,mBoardFadeOutCounter=-1,mWidth=800,mHeight=600,mLevel=1,advice=0;
 std::vector<Coin*> mCoins;
 bool IsScaryPotterDaveTalking(){return talking;}
 void DisplayAdvice(const char*,MessageStyle,AdviceType){++advice;}
 void CollectSunAt(int,int);
};
struct Coin {
 Board* mBoard;App* mApp;CoinType mType;bool mDead=false,mIsBeingCollected=false;
 int mPosX=100,mPosY=200,mWidth=40,mHeight=40,collections=0,sounds=0,value=0;
 Coin(Board& b,CoinType t):mBoard(&b),mApp(b.mApp),mType(t){b.mCoins.push_back(this);}
 bool IsSun();int GetSunValue();bool MouseHitTest(int,int,HitResult*);void MouseDown(int,int,int);
 bool IsPresentWithAdvice(){return false;}
 void PlayCollectSound(){++sounds;}
 void Collect(){mIsBeingCollected=true;++collections;value+=GetSunValue();}
};
// The runner appends the actual production implementations, not copies.
int main(){
 {Board b;Coin sun(b,CoinType::COIN_SUN),small(b,CoinType::COIN_SMALLSUN),large(b,CoinType::COIN_LARGESUN),gold(b,CoinType::COIN_GOLD),award(b,CoinType::COIN_FINAL_SEED_PACKET),seed(b,CoinType::COIN_USABLE_SEED_PACKET);
  b.CollectSunAt(500,500);assert(sun.collections==0);b.CollectSunAt(120,220);
  assert(sun.value==25&&small.value==15&&large.value==50&&b.advice==3);
  assert(gold.collections==0&&award.collections==0&&seed.collections==0);
  for(int i=0;i<10;++i)b.CollectSunAt(120,220);
  assert(sun.collections==1&&sun.sounds==1&&small.collections==1&&large.collections==1);
 }
 for(int guard=0;guard<9;++guard){Board b;Coin sun(b,CoinType::COIN_SUN);
  switch(guard){case 0:b.mPaused=true;break;case 1:b.mTimeStopCounter=1;break;case 2:b.mBoardFadeOutCounter=0;break;
   case 3:b.app.mGameScene=GameScenes::SCENE_LEVEL_INTRO;break;case 4:b.app.mGameScene=GameScenes::SCENE_ZOMBIES_WON;break;
   case 5:b.app.dialogs=1;break;case 6:b.talking=true;break;case 7:sun.mDead=true;break;case 8:sun.mIsBeingCollected=true;break;}
  b.CollectSunAt(120,220);assert(sun.collections==0&&sun.sounds==0);
 }
 for(auto tool:{CursorType::CURSOR_TYPE_NORMAL,CursorType::CURSOR_TYPE_PLANT_FROM_BANK,CursorType::CURSOR_TYPE_SHOVEL}){
  Board b;Coin sun(b,CoinType::COIN_SUN);b.cursor.mCursorType=tool;b.CollectSunAt(85,185);
  assert(sun.value==25&&b.cursor.mCursorType==tool);
 }
 {Board b;Coin sun(b,CoinType::COIN_SUN);sun.mPosX=-20;sun.mPosY=-20;b.CollectSunAt(-1,-1);assert(!sun.collections);}
 {Board b;Coin sun(b,CoinType::COIN_SUN);sun.mPosX=790;sun.mPosY=590;b.CollectSunAt(800,600);assert(!sun.collections);}
 {Board b;Coin sun(b,CoinType::COIN_SUN);sun.MouseDown(120,220,1);assert(sun.value==25);b.CollectSunAt(120,220);assert(sun.value==25);}
 std::cout<<"Sun hover: native hit areas, values, overlap, guards, tools and click passed\n";
}
