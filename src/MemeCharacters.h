#pragma once
#include <array>
class Plant; class Board; class Zombie; class Projectile;
namespace Sexy {class Graphics;class Color;}
namespace MemeCharacters {
inline constexpr int ShowoffLifetime=60*100; // Native simulation ticks, not wall-clock time.
struct Definition {int id,base,cost,unlock;const char* name;const char* shortName;const char* hint;const char* key;const char* description;};
inline constexpr std::array<Definition,17> Definitions{{
 {500,0,100,1,"红温豌豆","红温","满300怒气自动乱射80发","PEASHOOTER","普通攻击只有10%命中判定，其余豌豆飞行中反复上下飘，每次摆幅随机，不击退僵尸。每发增加20怒气，逐渐变红；满300自动乱射80发，不能手动释放。射完恢复绿色，不消耗生命。"},
 {501,3,50,4,"反咬坚果","反咬","被啃后反击 · 冷却3秒","WALL_NUT","被啃后反击面前的僵尸，造成80伤害。每3秒一次，不击退僵尸。"},
 {502,8,0,11,"显眼包蘑菇","显眼包","吸引邻路僵尸 · 存活60秒","PUFF_SHROOM","种下60秒后消失。近距离喷射孢子，每五秒吸引一只邻路普通步行僵尸。白天也能工作。不能吸引巨人和冰车。"},
 {503,1,50,2,"已读不回花","已读","平时产阳光 · 装死后反击","SUNFLOWER","平时每次生产50阳光。僵尸靠近时装死，经过后向后连发三颗豌豆。装死和反击时暂停生产，不能躲过碾压和巨人砸击。"},
 {504,52,125,8,"豌豆吐射手","倒飞","射手飞出去，豌豆留原地","SELF_THROWER","把自己弹出去撞击前方三格内的僵尸，再飞回原位。每次撞击造成80伤害，落地后休息三秒。冒险1-8解锁。"},
 {505,5,175,7,"退退退寒冰","退退退","冰弹命中后打退敌人","SNOW_PEA","每3秒射出一颗冰豌豆，命中后造成普通寒冰伤害，并让普通步行僵尸后退半格。不能推动巨人和车辆。"},
 {506,7,200,9,"复读双发","复读","邻居开火 · 跟着补射","REPEATER","自己不主动攻击。上下左右的邻居每发射一次，它就跟着补射一颗豌豆，最多暂存6发。复读之间不会互相触发。"},
 {507,4,75,6,"套娃土豆","套娃","炸完还有 · 最多三次","POTATO_MINE","准备时间和首次爆炸与土豆地雷相同。炸完留下小一号土豆，重新钻地准备6秒，最多爆炸三次。不会恢复生命，钻地时仍能被吃掉。"},
 {508,26,175,33,"弹幕仙人掌","弹幕","一口气六根刺 · 仍可对空","CACTUS","每轮连续射出六根原版尖刺，然后休息。遇到气球会伸长对空，升降期间不发射。"},
 {509,6,150,8,"蹭饭大嘴花","蹭饭","邻居开火 · 帮忙下饭","CHOMPER","保留吞咽和消化。上下左右的邻居开火时，帮它加快消化；每半秒最多蹭一次，每次少消化两秒。不回血，不吞巨人。"},
 {510,32,100,41,"甩锅卷心菜","甩锅","跳过前排 · 专挑最后一个","CABBAGE_PULT","把卷心菜甩给本路最后面的僵尸，越过前排。后排消失后重新选目标，保留原版抛物线、伤害和攻击间隔。"},
 {511,29,150,37,"蹦迪杨桃","蹦迪","每轮转方向 · 五向星弹","STARFRUIT","附近有敌人就射出五颗星星，每轮把发射方向转动一格。星星沿直线飞行，可以打到其他路，不追踪。"},
 {512,34,150,43,"爆米花玉米","爆米花","第三发黄油 · 定住后追打","KERNEL_PULT","前两轮投玉米，第三轮必投黄油。黄油落地后，如果本路还有被黄油定住的僵尸，就补射五粒玉米；没有目标则取消补射。"},
 {513,13,50,17,"嘴硬胆小菇","嘴硬","躲着攒话 · 安全后连喷","SCAREDY_SHROOM","平时照常远射，害怕时仍会缩起来。每躲一秒攒一发，最多六发，站起来后连续喷出。白天需要咖啡豆唤醒。"},
 {514,10,75,13,"打嗝大喷菇","打嗝","憋三秒 · 连打三个嗝","FUME_SHROOM","遇敌憋气三秒，随后连喷三口穿透烟雾，每口20伤害。没有目标时保留憋气进度；白天仍需唤醒。"},
 {515,28,150,35,"左右互搏裂荚","互搏","两个头吵架 · 左右轮流喷","SPLIT_PEA","本路任一侧有敌人就开始互喷：左右交替六发，每侧三发，随后休息2.5秒。使用普通豌豆，不推退敌人。"},
 {516,21,125,27,"烫脚地刺","烫脚","踩上来 · 烫得跳脚","SPIKEWEED","保留地刺攻击。每两秒把踩在上面的普通步行僵尸烫起一个小跳，额外造成20伤害，不横向推退。不能弹起巨人、车辆或水里的僵尸。"}
}};
constexpr const Definition* Find(int id){for(const auto& d:Definitions)if(d.id==id)return &d;return nullptr;}
constexpr bool Is(int id){return Find(id)!=nullptr;}
constexpr const Definition* ForBase(int base){for(const auto& d:Definitions)if(d.base==base)return &d;return nullptr;}
int Type(const Plant*);
bool Is(const Plant*);
bool Hiding(const Plant*);
bool Producing(const Plant*);
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
void OnImpact(Projectile*,Zombie*);
int ShotStyle(const Projectile*);
bool RestoreShotStyle(const Projectile*,int);
void ForgetShot(const Projectile*);
void UpdateShot(Projectile*);
bool RearmPotato(Plant*);
Zombie* PickTarget(Plant*,Zombie* nativeTarget);
bool ButterReady(const Plant*);
bool StarTarget(Plant*);
}
