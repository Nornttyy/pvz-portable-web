#pragma once
#include <array>
class Board; class Zombie; class Reanimation; class Plant; class Projectile;
namespace Sexy { class Graphics; class Image; }
namespace SandboxZombies {
struct Definition {int id,base;const char* name;const char* note;const char* art;int health,armor;};
// Previous original zombies are retired; the native roster is unchanged.
inline constexpr std::array<Definition,0> Definitions{};
constexpr const Definition* Find(int id){return nullptr;}
constexpr int Base(int id){auto* d=Find(id);return d?d->base:id;}
void Reset();void Forget(Zombie* zombie);void Assign(Zombie* zombie,int id);
void Tick(Board* board);void DrawPortrait(Sexy::Graphics* g,int x,int y,int w,int h,int id);
float Speed(const Zombie* zombie);int Damage(const Zombie* zombie,int damage);
bool ElectricHit(Zombie* zombie);void CombatDeath(Zombie* zombie);
void DrawEffects(Sexy::Graphics* g,Board* board,int row);
bool HasShot(const Projectile* shot);
bool DrawShot(Sexy::Graphics* g,const Projectile* shot);
bool Impact(Projectile* shot,Plant* plant);
Plant* CollisionTarget(Projectile* shot);
void ForgetShot(Projectile* shot);
void ForgetPlant(Plant* plant);
bool AttackSlowed(const Plant* plant);
void RefreshDamageArt(Zombie* zombie);
bool IsPhone(const Zombie* zombie);
void RecoverPhone(Zombie* zombie);
Sexy::Image* DetachedArmor(const Zombie* zombie);
}
