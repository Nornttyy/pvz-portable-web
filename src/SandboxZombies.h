#pragma once
#include <array>
class Board; class Zombie; class Reanimation; class Plant; class Projectile;
namespace Sexy { class Graphics; class Image; }
namespace SandboxZombies {
enum class Role { Battery, Shield, Turbine, Hook, Repair, Bolt, Drone, Jammer, Magnet, Holo };
struct Definition {int id,base;Role role;const char* name;const char* note;const char* art;int health,armor;};
inline constexpr std::array<Definition,10> Definitions{{
 {200,0,Role::Battery,"漏电背包僵尸","倒下留下废能芯 · 为附近同伴充电","leak-pack",320,0},
 {201,2,Role::Shield,"焊板盾僵尸","焊板护盾 · 破损后失去格挡","weld-shield",360,750},
 {202,0,Role::Turbine,"涡轮靴僵尸","蓄力冲刺 · 过热后减速","turbine-boot",280,0},
 {203,0,Role::Hook,"拖缆钩手","拉断附近根网 · 钩索可被打断","cable-hook",330,0},
 {204,4,Role::Repair,"扳手维修僵尸","修复机械护甲 · 破桶后失效","wrench-tech",320,650},
 {205,0,Role::Bolt,"螺栓投手","停步瞄准 · 远程螺栓","bolt-thrower",300,0},
 {206,0,Role::Drone,"嗡鸣浮空僵尸","悬浮移动 · 优先躲开地面陷阱","hover-drone",260,0},
 {207,0,Role::Jammer,"干扰天线僵尸","压制附近根网充能","jammer-aerial",360,0},
 {208,0,Role::Magnet,"磁暴回收僵尸","吸附芽弹 · 积蓄电磁脉冲","magnet-salvager",400,0},
 {209,0,Role::Holo,"全息诱饵僵尸","周期投影假身 · 首次受击免伤","holo-decoy",330,0},
}};
constexpr const Definition* Find(int id){return id>=200&&id<210?&Definitions[id-200]:nullptr;}
constexpr int Base(int id){auto* d=Find(id);return d?d->base:id;}
void Reset();void Forget(Zombie* zombie);void Assign(Zombie* zombie,int id);
void Tick(Board* board);void DrawPortrait(Sexy::Graphics* g,int x,int y,int w,int h,int id);
float Speed(const Zombie* zombie);int Damage(const Zombie* zombie,int damage);
bool ArcHit(Zombie* zombie,int damage=0);
bool IsMechanical(const Zombie* zombie);
bool IsResinSlowed(const Zombie* zombie);
void ApplyResin(Zombie* zombie,int ticks);
bool IsJamming(const Plant* relay);
bool HookRootCable(Board* board,Zombie* hooker,Plant* target);
bool Intercept(Projectile* shot);
// Zombie::DropLoot is the one native death hook shared by all causes of
// damage.  It hands mechanical-core credit to the plant-side root network.
void CombatDeath(Zombie* zombie);
void DrawEffects(Sexy::Graphics* g,Board* board,int row);
bool HasShot(const Projectile* shot);
bool DrawShot(Sexy::Graphics* g,const Projectile* shot);
bool Impact(Projectile* shot,Plant* plant);
Plant* CollisionTarget(Projectile* shot);
void ForgetShot(Projectile* shot);
void ForgetPlant(Plant* plant);
void RefreshDamageArt(Zombie* zombie);
Sexy::Image* DetachedArmor(const Zombie* zombie);
}
