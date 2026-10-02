#pragma once
namespace AlmanacPlantLayout {
struct Box {int x,y,w,h;constexpr bool Contains(int px,int py)const{return px>=x&&px<x+w&&py>=y&&py<y+h;}};
inline constexpr int Columns=8,Rows=6,PerPage=Columns*Rows;
inline constexpr int Extras[]{52,53,51,49};
inline constexpr float Scale(bool){return 1.0f;}
inline constexpr int ExtraSlot(int seed){for(int i=0;i<4;++i)if(Extras[i]==seed)return i;return -1;}
inline constexpr int Page(int seed,bool expanded){return seed>=0&&seed<=48?0:expanded&&ExtraSlot(seed)>=0?1:-1;}
inline constexpr bool Visible(int seed,bool expanded,int page){return page>=0&&Page(seed,expanded)==page;}
inline constexpr Box Card(int seed,bool expanded){
 if(seed==48)return {20,23,34,46}; // Original imitater badge, not an extra card.
 const int page=Page(seed,expanded);if(page<0)return {-100,-100,0,0};
 const int slot=page==0?seed:ExtraSlot(seed);
 // Match the eight printed columns in the ORIGINAL book background exactly.
 return {26+slot%Columns*52,92+slot/Columns*78,50,70};
}
inline constexpr Box Previous{258,567,44,26},Next{386,567,44,26};
inline constexpr int TurnAt(int x,int y,int page,int count){return page>0&&Previous.Contains(x,y)?-1:page+1<count&&Next.Contains(x,y)?1:0;}
}
