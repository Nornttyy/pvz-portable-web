#pragma once
#include <array>
class Plant;
class Board;
class Projectile;
class Zombie;
namespace Sexy { class Graphics; }
namespace SandboxPlants {
enum class Element { Fire, Ice, Alternating, Native };
struct Definition { int id, base; Element element; const char* name; const char* note; const char* art=nullptr; int rate=0,damage=20; float scale=1.0f; };
inline constexpr std::array<Definition,17> Definitions{{
    {100,0,Element::Fire,"火焰豌豆","火球攻击 · 小范围伤害"},
    {101,7,Element::Ice,"双发寒冰","连续两发 · 寒冰减速"},
    {102,7,Element::Fire,"双发火焰","连续两发 · 火球攻击"},
    {103,18,Element::Ice,"三线寒冰","三路攻击 · 寒冰减速"},
    {104,18,Element::Fire,"三线火焰","三路攻击 · 火球攻击"},
    {105,40,Element::Ice,"寒冰机枪","连续四发 · 寒冰减速"},
    {106,40,Element::Fire,"火焰机枪","连续四发 · 火球攻击"},
    {107,7,Element::Alternating,"冰火双发","冰火交替 · 火焰会解除减速"},
    {108,0,Element::Native,"回声花","两次声波 · 穿过整路敌人","echo-lily",240,18},
    {110,1,Element::Native,"节拍花","生产阳光 · 加快周围植物攻击","rhythm-flower"},
    {111,8,Element::Native,"雷电蘑菇","独立远程 · 成长后更强 · 成熟跳电","storm-mushroom-0",180,24},
    {112,0,Element::Native,"电能豌豆","电击跳向附近两只敌人","electric-pea",160,20},
    {113,0,Element::Native,"小豌豆","体型小 · 攻速快 · 单发伤害低","tiny-pea",75,10,0.72f},
    {114,0,Element::Native,"重炮豌豆","慢速重击 · 推退敌人","heavy-pea",360,65,1.04f},
    {115,0,Element::Native,"散射豌豆","三发散射 · 攻击相邻三路","scatter-pea",180,14},
    {116,0,Element::Native,"追击豌豆","子弹转向 · 自动追击敌人","seeker-pea",180,18},
    {117,0,Element::Native,"腐化豌豆","命中后持续伤害 · 再次命中刷新","acid-pea",180,12},
}};
constexpr const Definition* Find(int id) {
    for(const auto& d:Definitions)if(d.id==id)return &d;
    return nullptr;
}
constexpr int Base(int id) { const auto* d=Find(id);return d?d->base:id; }
constexpr Element ShotElement(int id,int shot) {
    const auto* d=Find(id);
    return d&&d->element==Element::Alternating?(shot%2?Element::Fire:Element::Ice):d?d->element:Element::Fire;
}
void Reset();
void Forget(Plant* plant);
void Assign(Plant* plant,int id);
int Type(const Plant* plant);
int GrowthStage(const Plant* plant);
bool IsCustom(const Plant* plant);
// 0 = native shot, 1 = ice, 2 = fire. Called once for each emitted pea.
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
}
