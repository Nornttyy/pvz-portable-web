#pragma once
#include <array>
#include <cmath>
namespace CoinPlantRules {
inline constexpr int Shooter=530,Flower=531,ShooterBase=50,FlowerBase=38;
inline constexpr int ShooterCost=150,FlowerCost=200,Recharge=750,Unlock=12;
inline constexpr int ShotInterval=150,FlowerInterval=1000,Limit=50,OrbitPeriod=600;
inline constexpr int Silver=1,Gold=2,Diamond=3,FirstShot=340;
constexpr bool IsPlant(int id){return id==Shooter||id==Flower;}
constexpr bool Kind(int kind){return kind>=Silver&&kind<=Diamond;}
constexpr bool Shot(int style){return style>=FirstShot&&style<FirstShot+3;}
constexpr int Style(int kind){return FirstShot+kind-1;}
constexpr int ShotKind(int style){return style-FirstShot+1;}
// Ammunition prices, not pickup values. Native wallet units equal 10 displayed coins.
constexpr int Units(int kind,bool flower=false){return kind==Silver?1:kind==Gold?(flower?2:1):kind==Diamond?(flower?10:5):0;}
constexpr int Damage(int kind){return kind==Silver?80:kind==Gold?400:kind==Diamond?4000:0;}
constexpr int Choose(int roll,bool /*flower*/){return roll<5?Diamond:roll<35?Gold:Silver;}
constexpr int Interval(int id){return id==Flower?FlowerInterval:ShotInterval;}
struct State {int id=0,delay=0,phase=0,pending=0;std::array<int,Limit> coins{};std::array<unsigned,Limit> touching{};};
constexpr int Count(const State& s){int n=0;for(int k:s.coins)n+=k!=0;return n;}
constexpr bool Valid(const State& s){
 if(!IsPlant(s.id)||s.delay<0||s.delay>Interval(s.id)||s.phase<0||s.phase>=OrbitPeriod||s.pending<0||s.pending>Diamond)return false;
 for(int i=0;i<Limit;++i)if(s.coins[i]<0||s.coins[i]>Diamond||(s.id==Shooter&&s.coins[i])||(!s.coins[i]&&s.touching[i]))return false;
 return true;
}
struct Point {float x,y;};
inline Point Orbit(int slot,int phase,float rowHeight=100){
 const int ring=slot%5;const float angle=6.28318530718f*(phase/float(OrbitPeriod)+(slot/5*3%10)/10.f)+ring*.43f;
 return {40+(50+ring*17.5f)*std::cos(angle),40+(55+ring*19.5f)*(rowHeight/100)*std::sin(angle)};
}
}
