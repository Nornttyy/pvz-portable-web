#include "CoinPlants.h"
#include "ConstEnums.h"
#include <cassert>
#include <map>
#include <algorithm>
#include <iostream>
enum FoleyType{FOLEY_SPLAT,FOLEY_SPAWN_SUN,FOLEY_THROW};
enum PlantOnBungeeState{NOT_ON_BUNGEE,GETTING_GRABBED_BY_BUNGEE};
enum PlantWeapon{WEAPON_PRIMARY};
struct Rect{int mX=0,mY=0,mWidth=0,mHeight=0;};
struct Player{int mCoins=0;void AddCoins(int n){assert(mCoins+n>=0);mCoins+=n;}};
class Reanimation{public:bool TrackExists(const char*){return true;}void PlayReanim(const char*,ReanimLoopType,int,float){}};
struct App{Player player;Player* mPlayerInfo=&player;Reanimation head;Reanimation* ReanimationTryToGet(int){return &head;}void PlayFoley(FoleyType){}} app;
App* gLawnApp=&app;bool gSandboxEnabled=false;
class Zombie{public:bool mDead=false,mMindControlled=false,held=false,flying=false;Rect rect{500,200,45,80};int damage=0;
 bool IsOnBoard(){return true;}bool IsDeadOrDying(){return mDead;}bool EffectedByDamage(int){return !flying;}Rect GetZombieRect(){return rect;}void TakeDamage(int n,unsigned){damage+=n;}};
class Projectile{public:bool mDead=false;int style=0,mMotionType=0,mDamageRangeFlags=0;float mVelX=0,mVelY=0;App* mApp=&app;
 void Die(){mDead=true;}unsigned GetDamageFlags(Zombie*){return 0;}};
class Plant{public:App* mApp=&app;Board* mBoard=nullptr;int custom=0,mRow=0,mX=100,mY=200,mPlantCol=1,mRenderOrder=100,mHeadReanimID=1,mPlantHealth=300;SeedType mSeedType=SEED_PEASHOOTER;
 bool mDead=false,mSquished=false,mIsAsleep=false,lifted=false;PlantOnBungeeState mOnBungeeState=NOT_ON_BUNGEE;Zombie* target=nullptr;
 bool NotOnGround(){return lifted;}bool IsInPlay(){return true;}Zombie* FindTargetZombie(int,PlantWeapon){return target;}};
struct Plants:std::vector<Plant*>{using std::vector<Plant*>::operator=;unsigned DataArrayGetID(Plant* p){return unsigned(std::find(begin(),end(),p)-begin()+1);}Plant* DataArrayTryToGet(unsigned n){return n&&n<=size()?(*this)[n-1]:nullptr;}};
class Board{public:bool mPaused=false;Plants mPlants;std::vector<Zombie*> mZombies;struct Shots{int mSize=0,mMaxSize=1024;std::vector<Projectile> values;}mProjectiles;
 void ShowCoinBank(){}Projectile* AddProjectile(float,float,int,int,ProjectileType type){assert(type==PROJECTILE_SPIKE);++mProjectiles.mSize;return &mProjectiles.values.emplace_back();}};
float PlantDrawHeightOffset(Board*,Plant*,SeedType,int,int){return 0;}
namespace MemeAdventure{bool enabled=true;bool RosterEnabled(){return enabled;}}
namespace MemeCharacters{int Type(const Plant* p){return p->custom;}int ShotStyle(const Projectile* p){return p->style;}bool RestoreShotStyle(Projectile* p,int style){p->style=style;return true;}}
namespace SandboxZombies{bool IsHeld(Zombie* z){return z->held;}}
namespace SandboxArt{bool TrackPoint(Reanimation*,const char*,float,float,float,float,float& x,float& y){x=58;y=34;return true;}}
namespace Sexy{int roll=99,calls=0;int Rand(int){++calls;return roll;}}
namespace CoinPlants{
using namespace CoinPlantRules;
namespace {std::map<const Plant*,State> states;Save pending;int fired[3]{},hits[3]{};
#include "coin-helpers.inc"
}
#include "coin-production.inc"
}
void tick(Plant& p,int n){while(n--)CoinPlants::Update(&p);}
int main(){
 using namespace CoinPlantRules;using namespace CoinPlants;
 int shotOdds[4]{},flowerOdds[4]{};
 for(int roll=0;roll<100;++roll){++shotOdds[Choose(roll,false)];++flowerOdds[Choose(roll,true)];}
 assert(shotOdds[1]==69&&shotOdds[2]==30&&shotOdds[3]==1);
 assert(flowerOdds[1]==65&&flowerOdds[2]==30&&flowerOdds[3]==5);
 for(bool flower:{false,true})for(int roll=0;roll<100;++roll){
  Reset();Board b;Zombie z;Plant p;p.mBoard=&b;p.custom=flower?Flower:Shooter;p.target=&z;b.mPlants={&p};app.player.mCoins=200;Sexy::roll=roll;
  const int interval=Interval(p.custom),kind=Choose(roll,flower);tick(p,interval-1);assert(app.player.mCoins==200);tick(p,1);
  assert(app.player.mCoins==200-Units(kind));auto saved=Capture(&b);assert(saved.size()==1&&saved[0].state.delay==interval);
  if(flower)assert(Count(saved[0].state)==1&&saved[0].state.coins[0]==kind);
  else{assert(b.mProjectiles.mSize==1);auto& shot=b.mProjectiles.values[0];assert(shot.style==Style(kind));assert(Impact(&shot,&z)&&z.damage==Damage(kind)&&shot.mDead);}
 }
 // No money, no target, full projectile pool, suspension and pausing never debit.
 for(int scenario=0;scenario<6;++scenario){Reset();Board b;Zombie z;Plant p;p.custom=Shooter;p.mBoard=&b;p.target=&z;b.mPlants={&p};app.player.mCoins=100;
  if(scenario==0)app.player.mCoins=0;if(scenario==1)p.target=nullptr;if(scenario==2)b.mProjectiles.mSize=b.mProjectiles.mMaxSize;
  if(scenario==3)b.mPaused=true;if(scenario==4)p.mOnBungeeState=GETTING_GRABBED_BY_BUNGEE;if(scenario==5)p.mSquished=true;
  const int before=app.player.mCoins,shots=b.mProjectiles.mSize;tick(p,500);assert(app.player.mCoins==before&&b.mProjectiles.mSize==shots);
 }
 // Unaffordable rare rolls do not reroll or become a discounted/free silver coin.
 Reset();Board b;Zombie z;Plant p;p.mBoard=&b;p.custom=Shooter;p.target=&z;b.mPlants={&p};app.player.mCoins=1;Sexy::roll=0;Sexy::calls=0;
 tick(p,500);assert(app.player.mCoins==1&&b.mProjectiles.mSize==0&&Sexy::calls==1);auto saved=Capture(&b);assert(saved[0].state.pending==Diamond);
 Reset();Load(saved);Restore(&b);assert(Capture(&b)[0].state.pending==Diamond&&app.player.mCoins==1);
 app.player.mCoins=100;tick(p,1);assert(app.player.mCoins==0&&b.mProjectiles.mSize==1&&b.mProjectiles.values[0].style==Style(Diamond));
 // Full 100-coin ring, stable individual slots, contact damage once, no refunds on cleanup.
 Reset();p.custom=Flower;b.mZombies.clear();app.player.mCoins=1000;State ring;ring.id=Flower;ring.delay=FlowerInterval;ring.coins.fill(Silver);
 Load({{1,ring}});Restore(&b);tick(p,FlowerInterval*2);assert(Count(Capture(&b)[0].state)==100&&app.player.mCoins==1000);
 saved=Capture(&b);saved[0].state.delay=FlowerInterval;saved[0].state.phase=0;saved[0].state.coins.fill(0);saved[0].state.coins[0]=Gold;
 const auto at=Orbit(0,1);z.rect={p.mX+int(at.x)-2,p.mY+int(at.y)-2,4,4};z.damage=0;b.mZombies={&z};Load(saved);Restore(&b);tick(p,1);
 assert(z.damage==Damage(Gold)&&Count(Capture(&b)[0].state)==0&&app.player.mCoins==1000);tick(p,1);assert(z.damage==Damage(Gold));
 Forget(&p);assert(Capture(&b).empty()&&app.player.mCoins==1000);
 ring.coins[0]=4;assert(!Valid(ring));ring.coins[0]=1;ring.phase=OrbitPeriod;assert(!Valid(ring));ring.phase=0;ring.id=Shooter;assert(!Valid(ring));
 Reset();Load({{999,State{Flower,FlowerInterval,0,0,{}}}});Restore(&b);assert(Capture(&b).empty());
 for(int slot=0;slot<Limit;++slot)for(int phase=0;phase<OrbitPeriod;++phase){const auto a=Orbit(slot,phase),n=Orbit(slot,(phase+1)%OrbitPeriod);assert(std::hypot(a.x-n.x,a.y-n.y)<2);}
 std::cout<<"Coin plants: exact odds and debit, clocks, zero funds, pending rolls, save restore, cap and one-hit orbit contacts passed\n";
}
