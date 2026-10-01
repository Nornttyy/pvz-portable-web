#pragma once
#include <array>
class Board; class Zombie; class Reanimation; class Plant; class Projectile;
namespace Sexy { class Graphics; class Image; }
namespace SandboxZombies {
struct Definition {int id,base;const char* name;const char* note;const char* art;int health,armor,unlock;};
// IDs 200..211 remain retired. Never reinterpret old characters as Louis.
inline constexpr int Louis=212, LouisUnlock=3, Runner=213, RunnerUnlock=6;
inline constexpr int ConeWrap=214, ConeWrapUnlock=16, ConeHealth=370, ConeCount=7;
inline constexpr int ConeVisualCount=20;
inline constexpr int GiantImp=215,GiantImpUnlock=22,JawSmash=1043,JawTicks=90,JawImpact=35;
inline constexpr float ConeWrapSpeed=0.60f;
inline constexpr int RunIn=1040,RunBrake=1041,RunOut=1042,BrakeTicks=8;
inline constexpr float RunInSpeed=5.6f,RunOutSpeed=7.2f,RunAnimRate=68.0f;
inline constexpr std::array<Definition,4> Definitions{{
 {Louis,0,"路易十六","天生无头，照常走路啃咬。",nullptr,270,0,LouisUnlock},
 {Runner,0,"跑路僵尸","冲到后排，转身就跑。",nullptr,270,0,RunnerUnlock},
 {ConeWrap,2,"雪糕桶包裹我","20桶组成，7桶耐久。",nullptr,270,ConeCount*ConeHealth,ConeWrapUnlock},
 {GiantImp,24,"巨人小鬼","巨人头，小鬼身。一击秒杀植物。",nullptr,270,0,GiantImpUnlock}
}};
constexpr const Definition* Find(int id){for(const auto& d:Definitions)if(d.id==id)return &d;return nullptr;}
constexpr int Base(int id){auto* d=Find(id);return d?d->base:id;}
constexpr bool LouisWave(int level,int base,int wave){return level>=LouisUnlock&&base==0&&wave>=0&&wave%3==0;}
constexpr bool RunnerWave(int level,int base,int wave){return level>=RunnerUnlock&&base==0&&wave>=0&&wave%4==1;}
constexpr bool ConeWrapWave(int level,int base,int wave){return level>=ConeWrapUnlock&&base==2&&wave>=3&&wave%4==3;}
// Only regular land-lane wave entries qualify, NEVER native/thrown imps.
constexpr bool GiantImpWave(int level,int base,int wave){return level>=GiantImpUnlock&&base==0&&wave>=2&&wave%4==2;}
// Armor stays worth seven cones; the twenty visual parts are not extra HP.
constexpr int ConePartHealth(int armor,int part){const int hp=armor-part*ConeHealth;return hp<=0?0:hp>=ConeHealth?ConeHealth:hp;}
constexpr int ConeVisualHealth(int armor,int part){const int hp=(armor*ConeVisualCount-part*ConeCount*ConeHealth)/ConeCount;return hp<=0?0:hp>=ConeHealth?ConeHealth:hp;}
constexpr int ConeDamageStage(int health){return health>2*ConeHealth/3?0:health>ConeHealth/3?1:2;}
int Type(const Zombie*);
bool IsConeWrap(const Zombie*);
bool IsGiantImp(const Zombie*);
bool IsLouis(const Zombie*);
bool IsRunner(const Zombie*);
bool IsRunning(const Zombie*);
bool UpdateRunner(Zombie*);
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
Sexy::Image* DetachedHead(const Zombie* zombie);
}
