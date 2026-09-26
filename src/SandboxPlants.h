#pragma once
#include <array>
class Plant;
class Board;
class Projectile;
class Zombie;
namespace Sexy { class Graphics; }
namespace SandboxPlants {
// The expansion is one coherent faction: living garden technology.  A role is
// gameplay-facing (and intentionally independent of the native seed used for
// animation), so a visual reskin can never silently become a new unit.
enum class Element { Pulse, Arc, Resin, Prism, Native };
enum class Role { Generator, Shooter, Barrier, Relay, Arc, Magnet, Resin, Scout, Prism, Repair };
struct Definition { int id, base; Element element; Role role; const char* name; const char* note; const char* art; int rate=0,damage=20; float scale=1.0f; };
inline constexpr std::array<Definition,10> Definitions{{
    {100,1,Element::Native,Role::Generator,"日轮花","产阳光 · 为根网充能","sunwheel",0,0},
    {101,0,Element::Pulse,Role::Shooter,"脉芽荚","脉冲种子 · 共振三连","pulse-pod",170,20},
    {102,3,Element::Native,Role::Barrier,"棘壳蕨","近身反刺 · 共振自愈","thornshell",0,18},
    {103,8,Element::Native,Role::Relay,"菌缆菇","连接根网 · 点按启动共振","cable-cap",0,0},
    {104,18,Element::Arc,Role::Arc,"弧苞兰","三路电弧 · 连锁机械敌人","arc-orchid",185,20},
    {105,0,Element::Pulse,Role::Magnet,"磁喉捕手","吸走护甲 · 打断装置","magnet-maw",210,18},
    {106,0,Element::Resin,Role::Resin,"胶琥瓜","树脂胶团 · 减速目标","amber-gourd",230,20},
    {107,0,Element::Pulse,Role::Scout,"巡芽花","追击芽弹 · 优先前排","scout-bloom",180,16},
    {108,0,Element::Prism,Role::Prism,"棱光芦苇","蓄能光束 · 贯穿整行","prism-reed",0,0},
    {109,1,Element::Native,Role::Repair,"修植苔","修复邻格 · 清除干扰","repair-moss",0,0},
}};
constexpr const Definition* Find(int id) {
    for(const auto& d:Definitions)if(d.id==id)return &d;
    return nullptr;
}
constexpr int Base(int id) { const auto* d=Find(id);return d?d->base:id; }
void Reset();
void Forget(Plant* plant);
void Assign(Plant* plant,int id);
int Type(const Plant* plant);
bool IsCustom(const Plant* plant);
bool IsTech(const Plant* plant);
bool IsResonating(const Plant* plant);
// 0 = native pea. The native firing hook still calls this once per emitted pea.
int NextShot(Plant* plant);
void Tick(Board* board);
void DrawCard(Sexy::Graphics* g,int x,int y,int id);
void OnFired(Plant* plant,Projectile* shot,Zombie* target);
bool Impact(Projectile* shot,Zombie* target);
void ForgetShot(Projectile* shot);
void UpdateShot(Projectile* shot);
void DrawEffects(Sexy::Graphics* g,Board* board,int row);
void AdjustScale(const Plant* plant,float& x,float& y,float& sx,float& sy);
void AdjustShadow(const Plant* plant,float& x,float& y,float& scale);
bool DrawShot(Sexy::Graphics* g,const Projectile* shot);
bool HasShot(const Projectile* shot);
float ShotScale(const Projectile* shot);
int ShotRadius(const Projectile* shot);

// Root-network interface. It is intentionally owned by plants: zombies only
// request disruption through these narrow operations.  The view makes all
// placement and pause semantics independently testable without a browser.
enum class RootEnergySource { Sun, Damage, ScrapCore };
enum class RootTriggerResult { Fired, NotHub, NotFull, Paused };
struct RootNetworkView {
    int network=-1,members=0,charge=0,activeTicks=0,pulseSerial=0;
    bool linked=false,severed=false,jammed=false;
};
void RebuildRootNetworks(Board* board);
RootNetworkView RootNetwork(const Plant* plant);
bool RootLinked(const Plant* a,const Plant* b);
bool AddRootEnergy(Board* board,Plant* source,RootEnergySource sourceType,int amount=1);
RootTriggerResult TriggerRootResonance(Board* board,Plant* hub);
bool SeverRootCable(Board* board,Plant* member,int ticks);
bool RepairRootCable(Board* board,Plant* repairer,Plant* member);
void OnMechanicalCoreDestroyed(Board* board,Zombie* zombie);

// UI adapter: the existing field-operation tool clicks a Cablecap mushroom.
// 1 success, 0 no relay, -1 not charged, -2 disrupted, -3 paused.
int ActivateNetwork(Board* board,int col,int row);
int NetworkCharge(const Plant* relay);
}
