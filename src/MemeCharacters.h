#pragma once
#include <array>
class Plant; class Board; class Zombie; class Projectile;
namespace Sexy {class Graphics;class Color;}
namespace MemeCharacters {
inline constexpr int ReflectedShot=512;
inline constexpr int ShooterPea=519, ShooterProjectile=296;
// Logical IDs survive save migration; sunflower now occupies its native slot.
inline constexpr int TuckingSunflower=520;
inline constexpr int LongRepeater=521, WeakProjectile=297;
inline constexpr int RepeaterCount=50, RepeaterInterval=2, RepeaterRest=150;
inline constexpr int GatlingShooter=522, GatlingProjectile=298;
inline constexpr int GatlingInterval=10, GatlingHeatLimit=120, GatlingCooldown=350;
inline constexpr int SmallNut=523, SmallNutHealth=4000/5;
inline constexpr float SmallNutScale=0.6f;
// Seed-card recharge, separate from each character's combat cooldown.
constexpr int PlantingCooldown(int id){return id==501?1200:id==SmallNut?600:300;}
constexpr bool IsStraightShot(int style){return style==WeakProjectile||style==GatlingProjectile;}
constexpr int BaseShotStyle(int style){return style&(ReflectedShot-1);}
struct Definition {int id,base,cost,unlock;const char* name;const char* shortName;const char* hint;const char* key;const char* description;};
inline constexpr std::array<Definition,7> Definitions{{
 {500,0,100,1,"红温豌豆","红温","满300怒气 · 3秒乱射40发","PEASHOOTER","普通攻击只有10%命中判定，其余豌豆反复上下飘，每次摆幅随机，不击退。每发增加20怒气，逐渐变红；满300自动在3秒内乱射40发，范围限本行及上下各一行，不能手动释放。红温子弹减速25%，射完休息3秒，不消耗生命。"},
 {501,3,50,4,"反咬坚果","反咬","被啃后反击 · 冷却3秒","WALL_NUT","被啃后反击面前的僵尸，造成80伤害。每3秒一次，不击退僵尸。"},
 {519,52,125,8,"射手豌豆","射手豌豆","头是豌豆 · 发射射手","SHOOTER_PEA","豌豆当脑袋，射手当子弹。向前发射完整的小豌豆射手，本体留在原地；每1.5秒一发，造成20伤害，不击退。冒险1-8解锁。"},
 {520,1,50,2,"缩头乌葵","缩头乌葵","缩头让路 · 暂停产光","SUNFLOWER","僵尸靠近就缩头，让僵尸直接走过，不会挡路或被啃。缩头时暂停产光计时，恢复后继续，每次25阳光。替换向日葵。"},
 {521,7,200,9,"双----------双发射手","双----------双发射手","每轮连射50发 · 单发1伤害","REPEATER","每轮连续射出50颗低伤害豌豆，每颗造成1伤害。约1秒射完，之后休息1.5秒。保留普通豌豆的弹速与直线弹道，不击退。替换双发射手。"},
 {522,40,250,0,"加特林射手","加特林","每0.1秒1发 · 过热休息3.5秒","GATLING_PEA","每0.1秒射出1颗豌豆，造成15伤害。连续射击12秒（120发）后过热，停火冷却3.5秒，再自动恢复攻击。保留原版外观、弹速和升级方式，种在双----------双发射手上。"},
 {523,53,25,4,"小·坚果","小·坚果","800生命 · 冷却6秒","SMALL_NUT","生命800，为坚果的五分之一。冷却6秒，需要25阳光。只挡路，不反咬。冒险1-4解锁。"},
}};
// Retired IDs are migration-only, never playable definitions.
inline constexpr std::array<int,17> RetiredBases{8,1,0,5,7,4,26,6,32,29,34,13,10,28,21,18,17};
constexpr bool IsRetired(int id){return id>=502&&id<=518;}
constexpr int RetiredBase(int id){return IsRetired(id)?RetiredBases[id-502]:-1;}
constexpr const Definition* Find(int id){for(const auto& d:Definitions)if(d.id==id)return &d;return nullptr;}
constexpr bool Is(int id){return Find(id)!=nullptr;}
constexpr const Definition* ForBase(int base){for(const auto& d:Definitions)if(d.base==base)return &d;return nullptr;}
int Type(const Plant*);
bool Is(const Plant*);
bool Hiding(const Plant*);
bool Producing(const Plant*);
void Assign(Plant*,int);
void Forget(Plant*);
void Reset();
void Tick(Board*);
bool Activate(Plant*,int direction=0);
bool Click(Board*,int x,int y);
std::array<int,10> Save(const Plant*);
bool Restore(Plant*,const std::array<int,10>&);
int Data(const Plant*,int field);
void Tint(const Plant*,Sexy::Color&);
void Scale(const Plant*,float& x,float& y,float& sx,float& sy);
void Effects(Sexy::Graphics*,Board*,int row);
void Card(Sexy::Graphics*,int x,int y,int id);
void OnFired(Plant*,Projectile*);
bool CanHit(const Projectile*);
bool CanHitRow(const Projectile*,int row);
void OnImpact(Projectile*,Zombie*);
int ShotStyle(const Projectile*);
bool RestoreShotStyle(const Projectile*,int);
void ForgetShot(const Projectile*);
void UpdateShot(Projectile*);
bool RearmPotato(Plant*);
bool ReturnSquash(Plant*);
Zombie* PickTarget(Plant*,Zombie* nativeTarget);
bool ButterReady(const Plant*);
bool StarTarget(Plant*);
}
