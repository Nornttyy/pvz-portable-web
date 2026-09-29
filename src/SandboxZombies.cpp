// Retired original zombie implementations. Hooks pass through to native gameplay.
#include "SandboxZombies.h"
namespace SandboxZombies {
void Reset(){} void Forget(Zombie*){} void Assign(Zombie*,int){}
void Tick(Board*){} void DrawPortrait(Sexy::Graphics*,int,int,int,int,int){}
float Speed(const Zombie*){return 1.0f;} int Damage(const Zombie*,int damage){return damage;}
bool ElectricHit(Zombie*){return false;} void CombatDeath(Zombie*){}
void DrawEffects(Sexy::Graphics*,Board*,int){}
bool HasShot(const Projectile*){return false;} bool DrawShot(Sexy::Graphics*,const Projectile*){return false;}
bool Impact(Projectile*,Plant*){return false;} Plant* CollisionTarget(Projectile*){return nullptr;}
void ForgetShot(Projectile*){} void ForgetPlant(Plant*){} bool AttackSlowed(const Plant*){return false;}
void RefreshDamageArt(Zombie*){} Sexy::Image* DetachedArmor(const Zombie*){return nullptr;}
}
