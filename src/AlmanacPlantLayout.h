#pragma once
namespace AlmanacPlantLayout {
struct Box {int x,y,w,h;};
inline constexpr float Scale(bool expanded){return expanded?0.88f:1.0f;}
inline constexpr Box Card(int seed,bool expanded){
 if(seed==48)return {20,23,34,46}; // Original imitater badge, not an extra card.
 const int slot=expanded?(seed==52?8:seed==53?9:seed>=8?seed+2:seed):seed;
 return expanded?Box{26+slot%9*46,92+slot/9*76,44,62}:Box{26+slot%8*52,92+slot/8*78,50,70};
}
}
