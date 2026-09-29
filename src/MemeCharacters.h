#pragma once
#include <array>
class Plant; class Board; class Zombie; class Projectile;
namespace Sexy {class Graphics;class Color;}
namespace MemeCharacters {
struct Definition {int id,base,cost,unlock;const char* name;const char* shortName;const char* hint;const char* key;const char* description;};
inline constexpr std::array<Definition,5> Definitions{{
 {500,0,100,1,"红温豌豆","红温","点击降温 · 过热会自伤","PEASHOOTER","连续射击会升温并加快攻击。热量超过三成后，点击降温并推退近处敌人。满热自伤，停火五秒。"},
 {501,3,50,4,"顶顶坚果","顶顶","被啃后向前撞","WALL_NUT","被啃后向前顶一下，再弹回原位。撞击造成80伤害，轻推普通步行僵尸，间隔两秒。"},
 {502,8,0,11,"显眼包蘑菇","显眼包","近距离射击 · 吸引邻路僵尸","PUFF_SHROOM","近距离喷射孢子，每五秒吸引一只邻路普通步行僵尸。白天也能工作。不能吸引巨人和冰车。"},
 {503,1,50,2,"已读不回花","已读","平时产阳光 · 装死后反击","SUNFLOWER","平时生产阳光。僵尸靠近时装死，经过后向后连发三颗豌豆。装死和反击时暂停生产，不能躲过碾压和巨人砸击。"},
 {504,52,125,8,"豌豆吐射手","倒飞","射手飞出去，豌豆留原地","SELF_THROWER","把自己弹出去撞击前方三格内的僵尸，再飞回原位。每次撞击造成80伤害，落地后休息三秒。冒险1-8解锁。"}
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
}
