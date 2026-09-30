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
inline constexpr std::array<Definition,158> Definitions{{
    {500,0,Element::Native,"红温豌豆","满300怒气自动乱射80发"},
    {501,3,Element::Native,"反咬坚果","被啃后反击 · 冷却3秒"},
    {502,8,Element::Native,"显眼包蘑菇","吸引邻路僵尸 · 存活60秒"},
    {503,1,Element::Native,"已读不回花","平时产阳光 · 装死后反击"},
    {504,52,Element::Native,"豌豆吐射手","射手飞出去，豌豆留原地"},
    {505,5,Element::Native,"退退退寒冰","冰弹命中后打退敌人"},
    {506,7,Element::Native,"复读双发","邻居开火 · 跟着补射"},
    {507,4,Element::Native,"套娃土豆","炸完还有 · 最多三次"},
    {508,26,Element::Native,"弹幕仙人掌","一口气六根刺 · 仍可对空"},
    {509,6,Element::Native,"蹭饭大嘴花","邻居开火 · 帮忙下饭"},
    {510,32,Element::Native,"甩锅卷心菜","跳过前排 · 专挑最后一个"},
    {511,29,Element::Native,"蹦迪杨桃","每轮转方向 · 五向星弹"},
    {512,34,Element::Native,"爆米花玉米","第三发黄油 · 定住后追打"},
    {513,13,Element::Native,"嘴硬胆小菇","躲着攒话 · 安全后连喷"},
    {120,0,Element::Native,"红温豌豆","持续攻击升温 · 红温连发 · 喘气停火","native-power"},
    {121,1,Element::Native,"红温向日葵","生产升温 · 集中产出 · 休息恢复","native-power"},
    {122,3,Element::Native,"红温坚果","受伤升温 · 爆发推退 · 冷静后再发动","native-power"},
    {123,5,Element::Native,"红温寒冰","持续攻击升温 · 红温连发 · 喘气停火","native-power"},
    {124,7,Element::Native,"红温双发","持续攻击升温 · 红温连发 · 喘气停火","native-power"},
    {125,18,Element::Native,"红温三线","持续攻击升温 · 红温连发 · 喘气停火","native-power"},
    {126,40,Element::Native,"红温机枪","持续攻击升温 · 红温连发 · 喘气停火","native-power"},
    {127,23,Element::Native,"红温高坚果","受伤升温 · 爆发推退 · 冷静后再发动","native-power"},
    {302,2,Element::Native,"红温樱桃炸弹","发动时追加热浪推退","native-power"},
    {304,4,Element::Native,"红温土豆地雷","发动时追加热浪推退","native-power"},
    {306,6,Element::Native,"红温大嘴花","吞咬后快速消化","native-power"},
    {308,8,Element::Native,"红温小喷菇","持续攻击升温 · 爆发后休息","native-power"},
    {309,9,Element::Native,"红温阳光菇","生产升温 · 周期额外产出","native-power"},
    {310,10,Element::Native,"红温大喷菇","持续攻击升温 · 爆发后休息","native-power"},
    {311,11,Element::Native,"红温墓碑吞噬者","发动时追加热浪推退","native-power"},
    {312,12,Element::Native,"红温魅惑菇","发动时追加热浪推退","native-power"},
    {313,13,Element::Native,"红温胆小菇","持续攻击升温 · 爆发后休息","native-power"},
    {314,14,Element::Native,"红温寒冰菇","发动时追加热浪推退","native-power"},
    {315,15,Element::Native,"红温毁灭菇","发动时追加热浪推退","native-power"},
    {316,16,Element::Native,"红温睡莲","守护近邻 · 受伤热浪","native-power"},
    {317,17,Element::Native,"红温窝瓜","发动时追加热浪推退","native-power"},
    {319,19,Element::Native,"红温缠绕海草","发动时追加热浪推退","native-power"},
    {320,20,Element::Native,"红温火爆辣椒","发动时追加热浪推退","native-power"},
    {321,21,Element::Native,"红温地刺","持续攻击升温 · 爆发后休息","native-power"},
    {322,22,Element::Native,"红温火炬树桩","点火升温 · 增强经过的火球","native-power"},
    {324,24,Element::Native,"红温海蘑菇","持续攻击升温 · 爆发后休息","native-power"},
    {325,25,Element::Native,"红温路灯花","守护近邻 · 受伤热浪","native-power"},
    {326,26,Element::Native,"红温仙人掌","持续攻击升温 · 爆发后休息","native-power"},
    {327,27,Element::Native,"红温三叶草","发动时追加热浪推退","native-power"},
    {328,28,Element::Native,"红温裂荚射手","持续攻击升温 · 爆发后休息","native-power"},
    {329,29,Element::Native,"红温杨桃","持续攻击升温 · 爆发后休息","native-power"},
    {330,30,Element::Native,"红温南瓜头","受伤升温 · 推退近敌","native-power"},
    {331,31,Element::Native,"红温磁力菇","吸取后快速恢复 · 护卫近邻","native-power"},
    {332,32,Element::Native,"红温卷心菜投手","持续攻击升温 · 爆发后休息","native-power"},
    {333,33,Element::Native,"红温花盆","守护近邻 · 受伤热浪","native-power"},
    {334,34,Element::Native,"红温玉米投手","持续攻击升温 · 爆发后休息","native-power"},
    {335,35,Element::Native,"红温咖啡豆","发动时追加热浪推退","native-power"},
    {336,36,Element::Native,"红温大蒜","受伤升温 · 推退近敌","native-power"},
    {337,37,Element::Native,"红温叶子保护伞","守护近邻 · 受伤热浪","native-power"},
    {338,38,Element::Native,"红温金盏花","生产升温 · 周期额外产出","native-power"},
    {339,39,Element::Native,"红温西瓜投手","持续攻击升温 · 爆发后休息","native-power"},
    {341,41,Element::Native,"红温双子向日葵","生产升温 · 周期额外产出","native-power"},
    {342,42,Element::Native,"红温忧郁菇","持续攻击升温 · 爆发后休息","native-power"},
    {343,43,Element::Native,"红温香蒲","持续攻击升温 · 爆发后休息","native-power"},
    {344,44,Element::Native,"红温冰西瓜","持续攻击升温 · 爆发后休息","native-power"},
    {345,45,Element::Native,"红温吸金磁","吸取后快速恢复 · 护卫近邻","native-power"},
    {346,46,Element::Native,"红温地刺王","持续攻击升温 · 爆发后休息","native-power"},
    {347,47,Element::Native,"红温玉米加农炮","炮击升温 · 加快装填","native-power"},
    {128,0,Element::Native,"内卷豌豆","持续攻击成长 · 攻速逐渐提高","native-power"},
    {129,1,Element::Native,"内卷向日葵","生产成长 · 产出逐渐加快","native-power"},
    {130,3,Element::Native,"内卷坚果","挨打成长 · 定时修复","native-power"},
    {131,5,Element::Native,"内卷寒冰","持续攻击成长 · 攻速逐渐提高","native-power"},
    {132,7,Element::Native,"内卷双发","持续攻击成长 · 攻速逐渐提高","native-power"},
    {133,18,Element::Native,"内卷三线","持续攻击成长 · 攻速逐渐提高","native-power"},
    {134,40,Element::Native,"内卷机枪","持续攻击成长 · 攻速逐渐提高","native-power"},
    {135,23,Element::Native,"内卷高坚果","挨打成长 · 定时修复","native-power"},
    {350,2,Element::Native,"内卷樱桃炸弹","发动后回收阳光并加快原卡冷却","native-power"},
    {352,4,Element::Native,"内卷土豆地雷","发动后回收阳光并加快原卡冷却","native-power"},
    {354,6,Element::Native,"内卷大嘴花","吞咬成长 · 消化越来越快","native-power"},
    {356,8,Element::Native,"内卷小喷菇","攻击成长 · 原攻击逐渐加快","native-power"},
    {357,9,Element::Native,"内卷阳光菇","生产成长 · 原生产物加速","native-power"},
    {358,10,Element::Native,"内卷大喷菇","攻击成长 · 原攻击逐渐加快","native-power"},
    {359,11,Element::Native,"内卷墓碑吞噬者","发动后回收阳光并加快原卡冷却","native-power"},
    {360,12,Element::Native,"内卷魅惑菇","发动后回收阳光并加快原卡冷却","native-power"},
    {361,13,Element::Native,"内卷胆小菇","攻击成长 · 原攻击逐渐加快","native-power"},
    {362,14,Element::Native,"内卷寒冰菇","发动后回收阳光并加快原卡冷却","native-power"},
    {363,15,Element::Native,"内卷毁灭菇","发动后回收阳光并加快原卡冷却","native-power"},
    {364,16,Element::Native,"内卷睡莲","后勤成长 · 修复附近植物","native-power"},
    {365,17,Element::Native,"内卷窝瓜","发动后回收阳光并加快原卡冷却","native-power"},
    {367,19,Element::Native,"内卷缠绕海草","发动后回收阳光并加快原卡冷却","native-power"},
    {368,20,Element::Native,"内卷火爆辣椒","发动后回收阳光并加快原卡冷却","native-power"},
    {369,21,Element::Native,"内卷地刺","攻击成长 · 原攻击逐渐加快","native-power"},
    {370,22,Element::Native,"内卷火炬树桩","点火成长 · 火球逐渐增强","native-power"},
    {372,24,Element::Native,"内卷海蘑菇","攻击成长 · 原攻击逐渐加快","native-power"},
    {373,25,Element::Native,"内卷路灯花","后勤成长 · 修复附近植物","native-power"},
    {374,26,Element::Native,"内卷仙人掌","攻击成长 · 原攻击逐渐加快","native-power"},
    {375,27,Element::Native,"内卷三叶草","发动后回收阳光并加快原卡冷却","native-power"},
    {376,28,Element::Native,"内卷裂荚射手","攻击成长 · 原攻击逐渐加快","native-power"},
    {377,29,Element::Native,"内卷杨桃","攻击成长 · 原攻击逐渐加快","native-power"},
    {378,30,Element::Native,"内卷南瓜头","挨打成长 · 定时修复","native-power"},
    {379,31,Element::Native,"内卷磁力菇","工作成长 · 恢复加快","native-power"},
    {380,32,Element::Native,"内卷卷心菜投手","攻击成长 · 原攻击逐渐加快","native-power"},
    {381,33,Element::Native,"内卷花盆","后勤成长 · 修复附近植物","native-power"},
    {382,34,Element::Native,"内卷玉米投手","攻击成长 · 原攻击逐渐加快","native-power"},
    {383,35,Element::Native,"内卷咖啡豆","发动后回收阳光并加快原卡冷却","native-power"},
    {384,36,Element::Native,"内卷大蒜","挨打成长 · 定时修复","native-power"},
    {385,37,Element::Native,"内卷叶子保护伞","后勤成长 · 修复附近植物","native-power"},
    {386,38,Element::Native,"内卷金盏花","生产成长 · 原生产物加速","native-power"},
    {387,39,Element::Native,"内卷西瓜投手","攻击成长 · 原攻击逐渐加快","native-power"},
    {389,41,Element::Native,"内卷双子向日葵","生产成长 · 原生产物加速","native-power"},
    {390,42,Element::Native,"内卷忧郁菇","攻击成长 · 原攻击逐渐加快","native-power"},
    {391,43,Element::Native,"内卷香蒲","攻击成长 · 原攻击逐渐加快","native-power"},
    {392,44,Element::Native,"内卷冰西瓜","攻击成长 · 原攻击逐渐加快","native-power"},
    {393,45,Element::Native,"内卷吸金磁","工作成长 · 恢复加快","native-power"},
    {394,46,Element::Native,"内卷地刺王","攻击成长 · 原攻击逐渐加快","native-power"},
    {395,47,Element::Native,"内卷玉米加农炮","炮击成长 · 装填越来越快","native-power"},
    {136,0,Element::Native,"摆烂豌豆","休息蓄力 · 连续齐射","native-power"},
    {137,1,Element::Native,"摆烂向日葵","休息蓄力 · 一次产出四个阳光","native-power"},
    {138,3,Element::Native,"摆烂坚果","停止受伤后 · 慢慢修复","native-power"},
    {139,5,Element::Native,"摆烂寒冰","休息蓄力 · 连续齐射","native-power"},
    {140,7,Element::Native,"摆烂双发","休息蓄力 · 连续齐射","native-power"},
    {141,18,Element::Native,"摆烂三线","休息蓄力 · 连续齐射","native-power"},
    {142,40,Element::Native,"摆烂机枪","休息蓄力 · 连续齐射","native-power"},
    {143,23,Element::Native,"摆烂高坚果","停止受伤后 · 慢慢修复","native-power"},
    {398,2,Element::Native,"摆烂樱桃炸弹","发动时让周围敌人陷入冰缓","native-power"},
    {400,4,Element::Native,"摆烂土豆地雷","发动时让周围敌人陷入冰缓","native-power"},
    {402,6,Element::Native,"摆烂大嘴花","慢慢消化 · 修复自身","native-power"},
    {404,8,Element::Native,"摆烂小喷菇","延长攻击间隔 · 蓄力重击","native-power"},
    {405,9,Element::Native,"摆烂阳光菇","延长生产间隔 · 一次多份产出","native-power"},
    {406,10,Element::Native,"摆烂大喷菇","延长攻击间隔 · 蓄力重击","native-power"},
    {407,11,Element::Native,"摆烂墓碑吞噬者","发动时让周围敌人陷入冰缓","native-power"},
    {408,12,Element::Native,"摆烂魅惑菇","发动时让周围敌人陷入冰缓","native-power"},
    {409,13,Element::Native,"摆烂胆小菇","延长攻击间隔 · 蓄力重击","native-power"},
    {410,14,Element::Native,"摆烂寒冰菇","发动时让周围敌人陷入冰缓","native-power"},
    {411,15,Element::Native,"摆烂毁灭菇","发动时让周围敌人陷入冰缓","native-power"},
    {412,16,Element::Native,"摆烂睡莲","安静休息 · 治疗附近植物","native-power"},
    {413,17,Element::Native,"摆烂窝瓜","发动时让周围敌人陷入冰缓","native-power"},
    {415,19,Element::Native,"摆烂缠绕海草","发动时让周围敌人陷入冰缓","native-power"},
    {416,20,Element::Native,"摆烂火爆辣椒","发动时让周围敌人陷入冰缓","native-power"},
    {417,21,Element::Native,"摆烂地刺","延长攻击间隔 · 蓄力重击","native-power"},
    {418,22,Element::Native,"摆烂火炬树桩","周期蓄力 · 下一发火球强化","native-power"},
    {420,24,Element::Native,"摆烂海蘑菇","延长攻击间隔 · 蓄力重击","native-power"},
    {421,25,Element::Native,"摆烂路灯花","安静休息 · 治疗附近植物","native-power"},
    {422,26,Element::Native,"摆烂仙人掌","延长攻击间隔 · 蓄力重击","native-power"},
    {423,27,Element::Native,"摆烂三叶草","发动时让周围敌人陷入冰缓","native-power"},
    {424,28,Element::Native,"摆烂裂荚射手","延长攻击间隔 · 蓄力重击","native-power"},
    {425,29,Element::Native,"摆烂杨桃","延长攻击间隔 · 蓄力重击","native-power"},
    {426,30,Element::Native,"摆烂南瓜头","停止受伤后逐渐修复","native-power"},
    {427,31,Element::Native,"摆烂磁力菇","休息恢复 · 治疗近邻","native-power"},
    {428,32,Element::Native,"摆烂卷心菜投手","延长攻击间隔 · 蓄力重击","native-power"},
    {429,33,Element::Native,"摆烂花盆","安静休息 · 治疗附近植物","native-power"},
    {430,34,Element::Native,"摆烂玉米投手","延长攻击间隔 · 蓄力重击","native-power"},
    {431,35,Element::Native,"摆烂咖啡豆","发动时让周围敌人陷入冰缓","native-power"},
    {432,36,Element::Native,"摆烂大蒜","停止受伤后逐渐修复","native-power"},
    {433,37,Element::Native,"摆烂叶子保护伞","安静休息 · 治疗附近植物","native-power"},
    {434,38,Element::Native,"摆烂金盏花","延长生产间隔 · 一次多份产出","native-power"},
    {435,39,Element::Native,"摆烂西瓜投手","延长攻击间隔 · 蓄力重击","native-power"},
    {437,41,Element::Native,"摆烂双子向日葵","延长生产间隔 · 一次多份产出","native-power"},
    {438,42,Element::Native,"摆烂忧郁菇","延长攻击间隔 · 蓄力重击","native-power"},
    {439,43,Element::Native,"摆烂香蒲","延长攻击间隔 · 蓄力重击","native-power"},
    {440,44,Element::Native,"摆烂冰西瓜","延长攻击间隔 · 蓄力重击","native-power"},
    {441,45,Element::Native,"摆烂吸金磁","休息恢复 · 治疗近邻","native-power"},
    {442,46,Element::Native,"摆烂地刺王","延长攻击间隔 · 蓄力重击","native-power"},
    {443,47,Element::Native,"摆烂玉米加农炮","慢速装填 · 扩大炮击范围","native-power"},
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
