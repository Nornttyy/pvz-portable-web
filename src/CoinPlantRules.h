#pragma once
#include <array>
#include <cmath>
namespace CoinPlantRules {
inline constexpr int Shooter=530,Flower=531,ShooterBase=50,FlowerBase=38;
inline constexpr int ShooterCost=150,FlowerCost=50,Recharge=750,Unlock=12;
inline constexpr int ShotInterval=150,FlowerInterval=1000,Limit=100,OrbitPeriod=600;
inline constexpr int Silver=1,Gold=2,Diamond=3,FirstShot=340;
constexpr bool IsPlant(int id){return id==Shooter||id==Flower;}
constexpr bool Kind(int kind){return kind>=Silver&&kind<=Diamond;}
constexpr bool Shot(int style){return style>=FirstShot&&style<FirstShot+3;}
constexpr int Style(int kind){return FirstShot+kind-1;}
constexpr int ShotKind(int style){return style-FirstShot+1;}
// Native wallet stores tens of displayed coins: silver=10, gold=50, diamond=1000.
constexpr int Units(int kind){return kind==Silver?1:kind==Gold?5:kind==Diamond?100:0;}
constexpr int Damage(int kind){return kind==Silver?20:kind==Gold?60:kind==Diamond?300:0;}
constexpr int Choose(int roll,bool flower){return roll<(flower?5:1)?Diamond:roll<(flower?35:31)?Gold:Silver;}
constexpr int Interval(int id){return id==Flower?FlowerInterval:ShotInterval;}
struct State {int id=0,delay=0,phase=0,pending=0;std::array<int,Limit> coins{};};
constexpr int Count(const State& s){int n=0;for(int k:s.coins)n+=k!=0;return n;}
constexpr bool Valid(const State& s){
 if(!IsPlant(s.id)||s.delay<0||s.delay>Interval(s.id)||s.phase<0||s.phase>=OrbitPeriod||s.pending<0||s.pending>Diamond)return false;
 for(int k:s.coins)if(k<0||k>Diamond||(s.id==Shooter&&k))return false;
 return true;
}
struct Point {float x,y;};
inline Point Orbit(int slot,int phase){
 const int ring=slot/20;const float angle=6.28318530718f*(phase/float(OrbitPeriod)+slot%20/20.f)+ring*.43f;
 return {40+(60+ring*12)*std::cos(angle),40+(43+ring*10)*std::sin(angle)};
}
}
