#pragma once
#include <array>
namespace SandboxSceneRules {
// Keep the saved day=0 / pool=1 identifiers stable.
struct Scene {int background,level;const char* name;const char* resources;};
inline constexpr std::array<Scene,6> Scenes{{
 {0,8,"白天","DelayLoad_Background1"},{2,28,"泳池","DelayLoad_Background3"},
 {1,18,"黑夜","DelayLoad_Background2"},{3,38,"浓雾","DelayLoad_Background4"},
 {4,48,"屋顶","DelayLoad_Background5"},{5,48,"僵王屋顶","DelayLoad_Background6"}
}};
constexpr bool Valid(int map){return map>=0&&map<int(Scenes.size());}
constexpr bool Pool(int map){return map==1||map==3;}
constexpr bool Roof(int map){return map==4||map==5;}
constexpr int Rows(int map){return Pool(map)?6:5;}
constexpr int CellY(int col,int row,int map){return 80+row*((Pool(map)||Roof(map))?85:100)+(Roof(map)&&col<5?(5-col)*20:0)-(Roof(map)?10:0);}
constexpr int Cell(int x,int y,int map){
 if(!Valid(map)||x<40||x>=760)return -1;
 const int col=(x-40)/80,top=CellY(col,0,map),height=Pool(map)||Roof(map)?85:100;
 if(y<top||y>=top+Rows(map)*height)return -1;
 return (y-top)/height*9+col;
}
}
