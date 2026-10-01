#pragma once
#include <array>
#include "MemeCharacters.h"
class Plant;
class Board;
class Projectile;
class Zombie;
class Reanimation;
namespace Sexy { class Graphics; class Color; }
namespace SandboxPlants {
enum class Element { Fire, Ice, Alternating, Native };
struct Definition { int id, base; Element element; const char* name; const char* note; const char* art=nullptr; int rate=0,damage=20; float scale=1.0f; };
inline constexpr std::array<Definition,7> Definitions{{
    {500,0,Element::Native,"红温豌豆","满300怒气 · 3秒乱射40发"},
    {501,3,Element::Native,"反咬坚果","被啃后反击 · 冷却3秒"},
    {519,52,Element::Native,"射手豌豆","头是豌豆 · 发射射手"},
    {520,1,Element::Native,"缩头乌葵","缩头让路 · 暂停产光"},
    {521,7,Element::Native,"双----------双发射手","每轮连射50发 · 单发1伤害"},
    {522,40,Element::Native,"加特林射手","每0.1秒1发 · 过热休息3.5秒"},
    {523,53,Element::Native,"小·坚果","800生命 · 冷却6秒"},
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
// Portable per-instance state; never serialize raw pointers.
using PowerSave = std::array<int,10>;
PowerSave SavePower(const Plant* plant);
bool RestorePower(Plant* plant,const PowerSave& saved);
void RestoreRetired(Plant* plant,const PowerSave& saved);
void Reset();
void Forget(Plant* plant);
void Assign(Plant* plant,int id);
int EffectiveBase(const Plant* plant);
int Power(const Plant* plant);
bool NativeCanAct(const Plant* plant);
void NativeAction(Plant* plant);
int NativeProduction(Plant* plant);
int NativeCooldown(const Plant* plant,int ticks);
int NativeDamage(const Plant* plant,int damage);
void OneShot(Plant* plant,Zombie* exclude=nullptr);
void NativeTint(const Plant* plant,Sexy::Color& color);
void TorchPower(Plant* plant,Projectile* shot);
int ShotDamage(const Projectile* shot,int damage);
int ShotBlastRadius(const Projectile* shot,int radius);
int SaveShot(const Projectile* shot);
void RestoreShot(const Projectile* shot,int percent);
int Type(const Plant* plant);
int GrowthStage(const Plant* plant);
// Read-only state for native tooltips and real-engine QA: phase, heat, timer.
int HeatData(const Plant* plant,int field);
bool KeepsNativeBlink(const Plant* plant);
bool IsCustom(const Plant* plant);
// 0 = native shot, 1 = ice, 2 = fire. Called once for each emitted pea.
int NextShot(Plant* plant);
void Tick(Board* board);
void DrawCard(Sexy::Graphics* g,int x,int y,int id);
void DrawPeaHeadPreview(Sexy::Graphics* g,float x,float y,bool imitater=false);
void DrawTuckingPreview(Sexy::Graphics* g,float x,float y,bool imitater=false);
// Original reanimations, temporary colour overrides; native layers stay intact.
bool DrawBody(Sexy::Graphics* g,const Plant* plant,float x,float y,bool squished=false);
void OnFired(Plant* plant,Projectile* shot,Zombie* target);
bool Impact(Projectile* shot,Zombie* target);
void ForgetShot(Projectile* shot);
void UpdateShot(Projectile* shot);
void DrawEffects(Sexy::Graphics* g,Board* board,int row);
void AdjustScale(const Plant* plant,float& x,float& y,float& sx,float& sy);
void AdjustShadow(const Plant* plant,float& x,float& y,float& scale);
bool DrawShot(Sexy::Graphics* g,const Projectile* shot);
bool HasShot(const Projectile* shot);
bool UsesCustomShotArt(const Projectile* shot);
float ShotScale(const Projectile* shot);
int ShotRadius(const Projectile* shot);
}
