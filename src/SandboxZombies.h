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
inline constexpr int DodgePercent=70,LaneChangePercent=60,ForwardFlightPercent=30;
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
 {Louis,0,"路易十六","生命270 / 护甲0\n正常移速，啃食100伤害/秒\n出场无头，不因无头持续掉血\n\n{KEYWORD}脑子落在家里了，家也忘在哪了。报名啃脑培训，表格要求从头写起，他当场被判缺考。",nullptr,270,0,LouisUnlock},
 {Runner,0,"跑路僵尸","生命270 / 护甲0\n冲刺7格/秒 / 逃跑9格/秒\n刹车0.08秒，冲到后排即返回\n奔跑时不啃食植物\n\n{KEYWORD}收到草地有免费脑子的消息，冲进去才发现是自己的。为防止被自己吃掉，现已原路撤回。",nullptr,270,0,RunnerUnlock},
 {ConeWrap,2,"雪糕桶包裹我","生命270 / 护甲2590，合计2860\n外形20个桶，耐久相当于7个桶\n移速为普通60%，啃食100伤害/秒\n\n{KEYWORD}全家桶拒绝认他当亲戚，因为他没有全家，只有桶。体检时医生敲了半天，问：有人吗？里面回：有桶。",nullptr,270,ConeCount*ConeHealth,ConeWrapUnlock},
 {GiantImp,24,"巨人小鬼","生命270 / 护甲0\n下颚伤害：目标全部剩余生命\n前摇0.55秒 / 一轮动作0.9秒\n3-2起出现，不随巨人投掷\n\n{KEYWORD}小鬼报名当巨人，系统只批准了头。每次点头，植物就以为天黑了。下巴目前单独交房租。",nullptr,270,0,GiantImpUnlock},
 {Clever,0,"智斗僵尸","生命270 / 护甲0\n普速1.25倍 / 逃跑3倍\n躲弹70%，翻身1.6秒免普通子弹\n翻身换行60% / 前飞2格30%\n闪避间隔1.2秒；偷1株就跑\n入水自带泳圈\n\n{KEYWORD}脑子说走上路，下巴说走下路，身体决定先后空翻。偷到植物才想起来：我家没有草坪。",nullptr,270,0,CleverUnlock},
 {CleverCone,2,"路障智斗僵尸","生命270 / 护甲370，合计640\n普速1.25倍 / 逃跑3倍\n躲弹70%，翻身1.6秒免普通子弹\n翻身换行60% / 前飞2格30%\n闪避间隔1.2秒；偷1株就跑\n入水自带泳圈\n\n{KEYWORD}给路障报了补习班，考试时自己躲进路障。成绩出来，路障考了第一，他被判定为文具。",nullptr,270,ConeHealth,CleverConeUnlock},
 {GreenCone,2,"绿路障僵尸","生命270 / 护甲740，合计1010\n护甲为普通路障的2倍\n正常移速，啃食100伤害/秒\n\n{KEYWORD}给路障浇了三年水，终于绿了。他坚信再等两年会长出红绿灯，届时所有豌豆都得等红灯。",nullptr,270,2*ConeHealth,GreenConeUnlock},
 {ConeTower,2,"路障叠叠高僵尸","生命270 / 护甲7400，合计7670\n20个路障，每个370耐久\n移速为普通15%，啃食100伤害/秒\n无法游泳\n\n{KEYWORD}二十个路障轮流当头。走一步先开二十次会，最上面说通过，最下面说收到的时候，天已经亮了。",nullptr,270,TowerCones*ConeHealth,ConeTowerUnlock}
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
