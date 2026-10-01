#pragma once
#include <array>
#include "TinyPuffRules.h"
#include "NukeShroomRules.h"
class Plant; class Board; class Zombie; class Projectile;
namespace Sexy {class Graphics;class Color;}
namespace MemeCharacters {
inline constexpr int ReflectedShot=512;
inline constexpr int ShooterPea=519, ShooterProjectile=296;
// Logical IDs survive save migration; sunflower now occupies its native slot.
inline constexpr int TuckingSunflower=520;
inline constexpr int LongRepeater=521, WeakProjectile=297;
inline constexpr int RepeaterCount=50, RepeaterInterval=2, RepeaterRest=150;
inline constexpr int GatlingShooter=522, GatlingProjectile=298;
inline constexpr int GatlingInterval=10, GatlingHeatLimit=120, GatlingCooldown=350;
inline constexpr int SmallNut=523, SmallNutHealth=4000/5;
inline constexpr float SmallNutScale=0.6f;
inline constexpr int CactusPalm=524, PalmProjectile=299, CriticalPalmProjectile=300, PalmInterval=500;
inline constexpr int TinyPuff=525, TinyPuffProjectile=301;
inline constexpr int NukeShroom=NukeShroomRules::Id;
constexpr bool IsPalmShot(int style){return style==PalmProjectile||style==CriticalPalmProjectile;}
constexpr int PalmDamage(int style){return style==CriticalPalmProjectile?110:80;}
constexpr int PalmPush(int style,bool giant){return (giant?16:48)*(style==CriticalPalmProjectile?3:2)/2;}
// Seed-card recharge, separate from each character's combat cooldown.
constexpr int PlantingCooldown(int id){return id==501?1200:id==SmallNut?600:id==CactusPalm?750:id==TinyPuff?TinyPuffRules::Recharge:id==NukeShroom?NukeShroomRules::Recharge:300;}
constexpr bool IsStraightShot(int style){return style==WeakProjectile||style==GatlingProjectile||IsPalmShot(style);}
constexpr int BaseShotStyle(int style){return style&(ReflectedShot-1);}
struct Definition {int id,base,cost,unlock;const char* name;const char* shortName;const char* hint;const char* key;const char* description;};
inline constexpr std::array<Definition,10> Definitions{{
 {500,0,100,1,"红温豌豆","红温","满300怒气 · 3秒乱射40发","PEASHOOTER","生命300 / 单发伤害20\n普攻1.5秒/发，命中判定10%\n每发怒气+20，满300自动红温\n红温3秒40发，结束休息3秒\n红温范围：本行及上下各1行\n不耗血，不击退，不能手动释放\n\n{KEYWORD}上次打中僵尸，僵尸报警称被空气殴打。他急得把准星吃了，申请转职电风扇。现在风扇要求退货。"},
 {501,3,50,4,"反咬坚果","反咬","被啃后反击 · 冷却3秒","WALL_NUT","生命4000 / 反击伤害80\n被啃后反击，反击冷却3秒\n不击退；种植冷却另计\n\n{KEYWORD}被啃一口后，他连夜把食物链倒过来贴。第二天申请当僵尸，被拒：你太坚果了。于是他把拒信也咬了。"},
 {519,52,125,8,"射手豌豆","射手豌豆","头是豌豆 · 发射射手","SHOOTER_PEA","生命300 / 单发伤害20\n攻速1.5秒/发，不击退\n发射完整的小豌豆射手\n\n{KEYWORD}豌豆射手把工牌戴反了，头成了弹药，弹药成了同事。嘴里查出三个编制，目前正在申请扩招。"},
 {520,1,50,2,"缩头乌葵","缩头乌葵","缩头让路 · 暂停产光","SUNFLOWER","生命300 / 每次产25阳光\n首次1.5-6.25秒，之后11.75-12.5秒\n僵尸靠近就缩头让路，不挡路\n缩头时暂停产光，恢复后继续计时\n\n{KEYWORD}僵尸一来，他把头塞进工资条。主人问人呢，地下传来一句：您好，您种的向日葵已离职。请找土豆办理交接。"},
 {521,7,200,9,"双----------双发射手","双----------双发射手","每轮连射50发 · 单发1伤害","REPEATER","生命300 / 单发伤害1\n每轮50发，全中总伤害50\n间隔0.02秒，约1秒射完\n每轮结束休息1.5秒，不击退\n\n{KEYWORD}他说只射两发。第一发叫双，剩下四十九发叫----------双。数学老师看完，申请加入僵尸，并要求先吃他的脑子。"},
 {522,40,250,0,"加特林射手","加特林","每0.1秒1发 · 过热休息3.5秒","GATLING_PEA","生命300 / 单发伤害15\n攻速0.1秒/发，每秒10发\n连射12秒共120发后过热\n停火散热3.5秒，不击退\n种在双发射手上升级\n\n{KEYWORD}嘴是加特林，脑子是电饭煲。射满十二秒自动跳到保温档。僵尸掀开锅盖，发现里面坐着一颗正在生闷气的米。"},
 {523,53,25,4,"小·坚果","小·坚果","800生命 · 冷却6秒","SMALL_NUT","生命800，为反咬坚果的1/5\n攻击伤害0，只挡路，不反咬\n种植冷却6秒，花费25阳光\n\n{KEYWORD}被鼠标滚轮缩小后，发现房贷没变。现已申请让僵尸蹲着吃，以维护成年坚果的尊严。申请表被当成了被子。"},
 {524,26,125,33,"仙人的掌","仙人的掌","5秒一掌 · 击退僵尸","CACTUS","生命300 / 普通伤害80\n5秒一掌，20%暴击造成110伤害\n击退：普通0.6格 / 巨人0.2格\n暴击击退1.5倍：0.9格 / 0.3格\n能升高攻击气球僵尸\n\n{KEYWORD}仙人没来，掌先到了。掌门不让他进门，他把门掌了。现因掌嘴被取消掌门资格，改行掌管掌声。"},
 {525,8,0,11,"真·小喷菇","真·小喷菇","同格最多5只 · 冷却2秒","PUFF_SHROOM","生命300 / 单发伤害10\n体型为小喷菇的1/3，同格最多5只\n种植冷却2秒，花费0阳光\n保留原版短射程与攻速，白天睡觉\n\n{KEYWORD}小喷菇嫌名字不够小，把自己又缩了三倍。五只合租一个坑，中间的当房东，下面两只睡地板，左右两只负责证明这不是一粒灰。"},
 {526,15,250,20,"核爆菇","核爆菇","全屏五连爆 · 留下3×3大坑","DOOM_SHROOM","生命300 / 每次爆炸伤害1800\n全屏爆炸5次，间隔0.8秒\n留下3×3大坑，180秒后恢复\n坑内植物和底座一起消失\n250阳光，冷却30秒；白天睡觉\n\n{KEYWORD}他说只炸一下，另外四下是回声。园丁问为什么地也没了，他说正在给草坪办理退货，九格起退，不包回填。"},
}};
// Retired IDs are migration-only, never playable definitions.
inline constexpr std::array<int,17> RetiredBases{8,1,0,5,7,4,26,6,32,29,34,13,10,28,21,18,17};
constexpr bool IsRetired(int id){return id>=502&&id<=518;}
constexpr int RetiredBase(int id){return IsRetired(id)?RetiredBases[id-502]:-1;}
constexpr const Definition* Find(int id){for(const auto& d:Definitions)if(d.id==id)return &d;return nullptr;}
constexpr bool Is(int id){return Find(id)!=nullptr;}
constexpr const Definition* ForBase(int base){for(const auto& d:Definitions)if(d.base==base)return &d;return nullptr;}
int Type(const Plant*);
bool Is(const Plant*);
bool Hiding(const Plant*);
bool Producing(const Plant*);
bool IsPuff(const Plant*);
int PuffCount(Board*,int col,int row);
bool PuffMuzzle(const Plant*,float& x,float& y);
void Assign(Plant*,int);
void Forget(Plant*);
void Reset();
void Tick(Board*);
bool Activate(Plant*,int direction=0);
bool Click(Board*,int x,int y);
std::array<int,10> Save(const Plant*);
bool Restore(Plant*,const std::array<int,10>&);
int Data(const Plant*,int field);
void Tint(const Plant*,Sexy::Color&);
void Scale(const Plant*,float& x,float& y,float& sx,float& sy);
void Effects(Sexy::Graphics*,Board*,int row);
void Card(Sexy::Graphics*,int x,int y,int id);
void OnFired(Plant*,Projectile*);
bool CanHit(const Projectile*);
bool CanHitRow(const Projectile*,int row);
void OnImpact(Projectile*,Zombie*);
int ShotStyle(const Projectile*);
bool RestoreShotStyle(const Projectile*,int);
void ForgetShot(const Projectile*);
void UpdateShot(Projectile*);
bool RearmPotato(Plant*);
bool ReturnSquash(Plant*);
Zombie* PickTarget(Plant*,Zombie* nativeTarget);
bool ButterReady(const Plant*);
bool StarTarget(Plant*);
}
