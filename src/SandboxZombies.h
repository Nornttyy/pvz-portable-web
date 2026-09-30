#pragma once
#include <array>
class Board; class Zombie; class Reanimation; class Plant; class Projectile;
namespace Sexy { class Graphics; class Image; }
namespace SandboxZombies {
struct Definition {int id,base;const char* name;const char* note;const char* art;int health,armor;};
// IDs 200..211 remain retired. Never reinterpret old characters as Louis.
inline constexpr int Louis=212, LouisUnlock=3;
inline constexpr std::array<Definition,1> Definitions{{
 {Louis,0,"路易十六","天生无头，照常走路啃咬。",nullptr,270,0}
}};
constexpr const Definition* Find(int id){for(const auto& d:Definitions)if(d.id==id)return &d;return nullptr;}
constexpr int Base(int id){auto* d=Find(id);return d?d->base:id;}
constexpr bool LouisWave(int level,int base,int wave){return level>=LouisUnlock&&base==0&&wave>=0&&wave%3==0;}
int Type(const Zombie*);
bool IsLouis(const Zombie*);
bool Restore(Zombie*,int id);
void RestoreNative(Board* board);
void Reset();void Forget(Zombie* zombie);void Assign(Zombie* zombie,int id);
void Tick(Board* board);void DrawPortrait(Sexy::Graphics* g,int x,int y,int w,int h,int id);
float Speed(Zombie* zombie);int Damage(Zombie* zombie,int damage,unsigned flags=0);
bool IsRetreating(Zombie* zombie);
bool IsFeigning(Zombie* zombie);
bool IsResting(Zombie* zombie);
void ArmorBroken(Zombie* zombie);
void PoleLanded(Zombie* zombie);
// Explicit, serialized native phases; never store raw plant/zombie pointers.
inline constexpr int Held=1024,Returned=1025,Tripped=1026,Slipping=1027,Pinned=1028,Misdirected=1029;
inline constexpr int Hurried=1030,DoorDash=1031,BrakeSlide=1032,Airlift=1033,LaneStep=1034,AirDrop=1035;
bool HasInteraction(const Zombie*);
bool IsHeld(const Zombie*);
bool CatchForReturn(Plant*,Zombie*);
bool UpdateInteraction(Zombie*);
bool Staple(Zombie*);
bool Slip(Zombie*);
bool Misdirect(Zombie*);
void AdjustPose(Zombie* zombie,Reanimation* body);
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
