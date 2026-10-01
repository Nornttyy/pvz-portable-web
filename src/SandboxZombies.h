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
inline constexpr int Clever=216,CleverCone=217,CleverUnlock=23,CleverConeUnlock=26;
inline constexpr int CleverFlip=1044,CleverFlee=1045,FlipTicks=160,LegacyFlipTicks=90,DodgeRecovery=120;
inline constexpr int DodgePercent=70,LaneChangePercent=30,ForwardFlightPercent=10;
inline constexpr int ForwardFlightTag=21610,JawPoseTicks=110;
// Version the motion in a native saved field, so an older mid-air save can
// finish its original trajectory without a position/rotation jump.
inline constexpr int SlowFlipTag=21620,SlowForwardFlightTag=21621,FlipJawTicks=FlipTicks+20;
inline constexpr float ForwardFlightDistance=160.0f;
constexpr float FlipTravel(float t){return t*t*(3.0f-2.0f*t);}
constexpr float SlowFlipProgress(float t){
 if(t<=0)return 0;if(t>=1)return 1;
 if(t>.75f)return 1-SlowFlipProgress(1-t);
 if(t>=.25f)return .42f+(t-.25f)*.32f;
 // C1-continuous takeoff -> slow apex -> landing. The middle 0.8 seconds
 // covers only 16% of the action; all motion uses this one local clock.
 const float u=t*4;return u*(.375f+u*(.43f-.385f*u));
}
inline constexpr float CleverSpeed=1.25f,CleverFleeSpeed=3.0f;
inline constexpr int GreenCone=218,GreenConeUnlock=27,ConeTower=219,ConeTowerUnlock=32;
inline constexpr int TowerCones=20,TowerConeRise=6;
inline constexpr float ConeTowerSpeed=.15f;
inline constexpr std::array<Definition,8> Definitions{{
 {Louis,0,"路易十六","天生无头，照常走路啃咬。",nullptr,270,0,LouisUnlock},
 {Runner,0,"跑路僵尸","冲到后排，转身就跑。",nullptr,270,0,RunnerUnlock},
 {ConeWrap,2,"雪糕桶包裹我","20桶组成，7桶耐久。",nullptr,270,ConeCount*ConeHealth,ConeWrapUnlock},
 {GiantImp,24,"巨人小鬼","巨人头，小鬼身。一击秒杀植物。",nullptr,270,0,GiantImpUnlock},
 {Clever,0,"智斗僵尸","70%翻身闪弹，30%换路。翻身时10%前飞两格，偷完就跑。",nullptr,270,0,CleverUnlock},
 {CleverCone,2,"路障智斗僵尸","戴路障的智斗僵尸。入水套泳圈，偷完就跑。",nullptr,270,ConeHealth,CleverConeUnlock},
 {GreenCone,2,"绿路障僵尸","绿色路障，双倍耐久。",nullptr,270,2*ConeHealth,GreenConeUnlock},
 {ConeTower,2,"路障叠叠高僵尸","头顶20个路障。极慢，极耐打，无法游泳。",nullptr,270,TowerCones*ConeHealth,ConeTowerUnlock}
}};
constexpr const Definition* Find(int id){for(const auto& d:Definitions)if(d.id==id)return &d;return nullptr;}
constexpr int Base(int id){auto* d=Find(id);return d?d->base:id;}
constexpr bool WaterAllowed(int id){return id!=ConeTower;}
constexpr int ConeVariantWave(int level,int base,int wave,bool water){return base!=2||wave<1?-1:level>=ConeTowerUnlock&&wave%8==1&&!water?ConeTower:level>=GreenConeUnlock&&wave%4==0?GreenCone:-1;}
constexpr int TowerCount(int armor){return armor<=0?0:armor>=TowerCones*ConeHealth?TowerCones:(armor+ConeHealth-1)/ConeHealth;}
constexpr int TowerTopHealth(int armor){return TowerCount(armor)?armor-(TowerCount(armor)-1)*ConeHealth:0;}
constexpr bool LouisWave(int level,int base,int wave){return level>=LouisUnlock&&base==0&&wave>=0&&wave%3==0;}
constexpr bool RunnerWave(int level,int base,int wave){return level>=RunnerUnlock&&base==0&&wave>=0&&wave%4==1;}
constexpr bool ConeWrapWave(int level,int base,int wave){return level>=ConeWrapUnlock&&base==2&&wave>=3&&wave%4==3;}
// Only regular land-lane wave entries qualify, NEVER native/thrown imps.
constexpr bool GiantImpWave(int level,int base,int wave){return level>=GiantImpUnlock&&base==0&&wave>=2&&wave%4==2;}
constexpr int CleverWave(int level,int base,int wave){return wave>=1&&wave%4==0&&level>=CleverUnlock&&base==0?Clever:wave>=1&&wave%4==2&&level>=CleverConeUnlock&&base==2?CleverCone:-1;}
// Armor stays worth seven cones; the twenty visual parts are not extra HP.
constexpr int ConePartHealth(int armor,int part){const int hp=armor-part*ConeHealth;return hp<=0?0:hp>=ConeHealth?ConeHealth:hp;}
constexpr int ConeVisualHealth(int armor,int part){const int hp=(armor*ConeVisualCount-part*ConeCount*ConeHealth)/ConeCount;return hp<=0?0:hp>=ConeHealth?ConeHealth:hp;}
constexpr int ConeDamageStage(int health){return health>2*ConeHealth/3?0:health>ConeHealth/3?1:2;}
int Type(const Zombie*);
bool IsConeWrap(const Zombie*);
bool IsGiantImp(const Zombie*);
bool IsClever(const Zombie*);
bool IsDodging(const Zombie*);
bool UsesSlowFlip(const Zombie*);
bool IsForwardFlight(const Zombie*);
int FlipDuration(const Zombie*);
bool ShowsCleverJaw(const Zombie*);
bool SkipsProjectile(const Projectile*,const Zombie*);
bool DodgeProjectile(Projectile*,Zombie*);
bool StealPlant(Zombie*,Plant*);
bool UpdateClever(Zombie*);
void RefreshCleverRig(Zombie*);
void DrawCarriedPlant(Sexy::Graphics*,Zombie*);
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
