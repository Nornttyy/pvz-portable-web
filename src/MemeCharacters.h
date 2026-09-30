#pragma once
#include <array>
class Plant; class Board; class Zombie; class Projectile;
namespace Sexy {class Graphics;class Color;}
namespace MemeCharacters {
inline constexpr int ShowoffLifetime=60*100; // Native simulation ticks, not wall-clock time.
inline constexpr int ReflectedShot=512;
constexpr int BaseShotStyle(int style){return style&(ReflectedShot-1);}
struct Definition {int id,base,cost,unlock;const char* name;const char* shortName;const char* hint;const char* key;const char* description;};
inline constexpr std::array<Definition,19> Definitions{{
 {500,0,100,1,"红温豌豆","红温","满300怒气 · 3秒乱射50发","PEASHOOTER","普通攻击只有10%命中判定，其余豌豆反复上下飘，每次摆幅随机，不击退。每发增加20怒气，逐渐变红；满300自动在3秒内乱射50发，不能手动释放。红温子弹减速25%，射完休息3秒，不消耗生命。"},
 {501,3,50,4,"反咬坚果","反咬","被啃后反击 · 冷却3秒","WALL_NUT","被啃后反击面前的僵尸，造成80伤害。每3秒一次，不击退僵尸。"},
 {502,8,0,11,"显眼包蘑菇","显眼包","吸引邻路僵尸 · 存活60秒","PUFF_SHROOM","种下60秒后消失。近距离喷射孢子，每五秒吸引一只邻路普通步行僵尸。白天也能工作。不能吸引巨人和冰车。"},
 {503,1,50,2,"已读不回花","已读","平时产阳光 · 装死后反击","SUNFLOWER","平时每次生产25阳光，生产间隔缩短一半。僵尸靠近时装死，经过后向后连发三颗豌豆。装死和反击时暂停生产，不能躲过碾压和巨人砸击。"},
 {504,52,125,8,"豌豆吐射手","倒飞","射手飞出去，豌豆留原地","SELF_THROWER","把自己弹出去撞击前方三格内的僵尸，再飞回原位。每次撞击造成80伤害，落地后休息三秒。冒险1-8解锁。"},
 {505,5,175,7,"退退退寒冰","退退退","冰弹命中后打退敌人","SNOW_PEA","每3秒射出一颗冰豌豆，命中后造成普通寒冰伤害，并让普通步行僵尸后退半格。不能推动巨人和车辆。"},
 {506,7,200,9,"复读双发","复读","偷听邻居 · 学它吐什么","REPEATER","不主动攻击。邻居开火后偷听并模仿最近一次弹种：冰豌豆、火豌豆和孢子都能学，其余吐普通豌豆。最多暂存6发，复读之间不互相触发。"},
 {507,4,75,6,"套娃土豆","套娃","炸完还有 · 最多三次","POTATO_MINE","准备时间和首次爆炸与土豆地雷相同。炸完留下小一号土豆，重新钻地准备6秒，最多爆炸三次。不会恢复生命，钻地时仍能被吃掉。"},
 {508,26,175,33,"订书机仙人掌","订书机","前后两只 · 订在一起","CACTUS","尖刺命中普通步行僵尸时，若后面130像素内还有同路同伴，就把两者订在地上1.8秒。单只不生效、不重复刷新。保留原版对空和单发，不再六连射。"},
 {509,6,150,8,"退货大嘴花","退货","吃一口嫌弃 · 连人吐回","CHOMPER","吞下普通步行僵尸一秒后，把同一只活僵尸连盔甲吐回后方；撞到同伴会把它绊倒一秒。不复制、不回血、不额外伤害。吐完休息，巨人和特殊目标沿用原版规则。"},
 {510,32,100,41,"甩锅卷心菜","甩锅","甩给队尾 · 原路滚回来","CABBAGE_PULT","先砸本路队尾，落地后卷心菜向左滚回来，再撞一次就碎。先打后排再打前排，保持原版单次伤害。"},
 {511,29,150,37,"蹦迪杨桃","蹦迪","每轮转方向 · 五向星弹","STARFRUIT","附近有敌人就射出五颗星星，每轮把发射方向转动一格。星星沿直线飞行，可以打到其他路，不追踪。"},
 {512,34,150,43,"滑铲玉米","滑铲","黄油脚滑 · 撞翻同伴","KERNEL_PULT","第三发黄油让普通步行僵尸脚滑向后，撞到同伴会把它绊倒。全程不追加伤害，巨人和车辆只受原版黄油效果。不再追射五粒玉米。"},
 {513,13,50,17,"假导航胆小菇","假导航","自己缩起来 · 骗人走反路","SCAREDY_SHROOM","害怕缩下时，把身边同路的一只普通步行僵尸骗得转身往回走1.3秒。每6秒最多一次，白天仍睡觉。站起后正常射击，不再积攒连喷。"},
 {514,10,75,13,"打嗝大喷菇","打嗝","先吸一口 · 再嗝出去","FUME_SHROOM","憋气三秒时吸近前方普通步行僵尸，再打三个嗝将它们吹回；每口仍用原版穿透烟雾和20伤害。冻住的僵尸、巨人、车辆和水路不位移。白天仍需唤醒。"},
 {515,28,150,35,"杠精裂荚","杠精","你往右 · 我偏往左","SPLIT_PEA","自己不造豌豆。接住经过嘴边的友方豌豆，从另一张嘴反向吐出；保留冰火属性、伤害和原有命中判定。每颗最多反向一次，不能互相无限传球。摆在敌人后方可倒打前排。"},
 {516,21,125,27,"烫脚地刺","烫脚","踩上来 · 烫得跳脚","SPIKEWEED","保留地刺攻击。每两秒把踩在上面的普通步行僵尸烫起一个小跳，额外造成20伤害，不横向推退。不能弹起巨人、车辆或水里的僵尸。"},
 {517,18,325,23,"回旋镖三线","回旋镖","没打中就回来 · 三路回旋","THREEPEATER","向三路发射普通豌豆。没打中敌人的豌豆飞到草地右边后会掉头，返回时仍能伤害敌人。命中就消失，不重复穿透，不改变火炬转化。"},
 {518,17,75,21,"仰卧起坐窝瓜","起坐","砸完跳回去 · 最多三次","SQUASH","发现近处敌人后跳起砸下，每次600伤害。前两次落地后会跳回原格，再找目标；第三次落地消失。不回血，水里落地仍会消失。"}
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
bool ReturnSquash(Plant*);
Zombie* PickTarget(Plant*,Zombie* nativeTarget);
bool ButterReady(const Plant*);
bool StarTarget(Plant*);
}
