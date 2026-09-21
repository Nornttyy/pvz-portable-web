#pragma once
#include <array>
class Board; class Zombie; class Reanimation;
namespace Sexy { class Graphics; }
namespace SandboxZombies {
struct Definition {int id,base;const char* name;const char* note;const char* art;int health,armor;};
inline constexpr std::array<Definition,10> Definitions{{
 {200,2,"纸盒僵尸","纸盒破后短暂加速","parcel-zombie",270,300},
 {201,1,"助威僵尸","周期加快附近同伴的移动","bell-zombie",320,0},
 {202,0,"泡泡糖僵尸","靠近后降低植物攻速","gum-zombie",360,0},
 {203,4,"冰桶僵尸","铁桶完整时免受寒冰减速","ice-bucket-zombie",270,850},
 {204,0,"电能僵尸","首次受到电击后恢复生命并加速","battery-zombie",340,0},
 {205,0,"轻装僵尸","体型小 · 移速快 · 生命低","light-zombie",160,0},
 {206,2,"重甲路障","移动缓慢 · 护甲厚重","armored-cone-zombie",350,950},
 {207,4,"修理僵尸","周期修复附近同伴的护甲","repair-zombie",300,600},
 {208,0,"烟雾僵尸","周期进入烟雾 · 受到伤害减半","smoke-zombie",360,0},
 {209,0,"双子僵尸","倒下后出现两只小鬼","twin-zombie",380,0},
}};
constexpr const Definition* Find(int id){return id>=200&&id<210?&Definitions[id-200]:nullptr;}
constexpr int Base(int id){auto* d=Find(id);return d?d->base:id;}
void Reset();void Forget(Zombie* zombie);void Assign(Zombie* zombie,int id);
void Tick(Board* board);void DrawPortrait(Sexy::Graphics* g,int x,int y,int w,int h,int id);
float Speed(const Zombie* zombie);int Damage(const Zombie* zombie,int damage);
bool ElectricHit(Zombie* zombie);void CombatDeath(Zombie* zombie);
void DrawEffects(Sexy::Graphics* g,Board* board,int row);
}
