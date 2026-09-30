#pragma once
#include <array>
class Plant; class Board; class Zombie; class Projectile;
namespace Sexy {class Graphics;class Color;}
namespace MemeCharacters {
inline constexpr int ReflectedShot=512;
constexpr int BaseShotStyle(int style){return style&(ReflectedShot-1);}
struct Definition {int id,base,cost,unlock;const char* name;const char* shortName;const char* hint;const char* key;const char* description;};
inline constexpr std::array<Definition,2> Definitions{{
 {500,0,100,1,"红温豌豆","红温","满300怒气 · 3秒乱射50发","PEASHOOTER","普通攻击只有10%命中判定，其余豌豆反复上下飘，每次摆幅随机，不击退。每发增加20怒气，逐渐变红；满300自动在3秒内乱射50发，不能手动释放。红温子弹减速25%，射完休息3秒，不消耗生命。"},
 {501,3,50,4,"反咬坚果","反咬","被啃后反击 · 冷却3秒","WALL_NUT","被啃后反击面前的僵尸，造成80伤害。每3秒一次，不击退僵尸。"},
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
