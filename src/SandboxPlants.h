#pragma once
#include <array>
class Plant;
class Board;
namespace Sexy { class Graphics; }
namespace SandboxPlants {
enum class Element { Fire, Ice, Alternating };
struct Definition { int id, base; Element element; const char* name; const char* note; };
inline constexpr std::array<Definition,8> Definitions{{
    {100,0,Element::Fire,"火焰豌豆","火球攻击 · 小范围伤害"},
    {101,7,Element::Ice,"双发寒冰","连续两发 · 寒冰减速"},
    {102,7,Element::Fire,"双发火焰","连续两发 · 火球攻击"},
    {103,18,Element::Ice,"三线寒冰","三路攻击 · 寒冰减速"},
    {104,18,Element::Fire,"三线火焰","三路攻击 · 火球攻击"},
    {105,40,Element::Ice,"寒冰机枪","连续四发 · 寒冰减速"},
    {106,40,Element::Fire,"火焰机枪","连续四发 · 火球攻击"},
    {107,7,Element::Alternating,"冰火双发","冰火交替 · 火焰会解除减速"},
}};
constexpr const Definition* Find(int id) {
    return id>=100 && id<108 ? &Definitions[id-100] : nullptr;
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
bool IsCustom(const Plant* plant);
// 0 = native shot, 1 = ice, 2 = fire. Called once for each emitted pea.
int NextShot(Plant* plant);
void Tick(Board* board);
void DrawCard(Sexy::Graphics* g,int x,int y,int id);
}
