#pragma once
#include <array>
class Plant; class Board; class Zombie; class Projectile;
namespace Sexy {class Graphics;class Color;}
namespace MemeCharacters {
struct Definition {int id,base,cost,unlock;const char* name;const char* shortName;const char* hint;};
inline constexpr std::array<Definition,4> Definitions{{
 {500,0,150,3,"红温豌豆","红温","点击降温 · 过热会自伤"},
 {501,3,100,6,"甩锅坚果","甩锅","点上半向上 · 下半向下"},
 {502,8,125,8,"显眼包蘑菇","显眼包","吸引邻路普通步行僵尸"},
 {503,1,125,8,"已读不回花","已读","装死放行 · 回身三连击"}
}};
constexpr const Definition* Find(int id){for(const auto& d:Definitions)if(d.id==id)return &d;return nullptr;}
constexpr bool Is(int id){return Find(id)!=nullptr;}
int Type(const Plant*);
bool Is(const Plant*);
bool Hiding(const Plant*);
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
