#pragma once
#include <algorithm>
#include <cstdint>
#include <cmath>
namespace NukeShroomRules {
inline constexpr int Id=526, Cost=250, Recharge=3000, Pulses=5, Interval=80;
inline constexpr int CraterLife=18000, CraterMarker=52600, Size=3;
struct Area {int col,row;constexpr bool Contains(int c,int r)const{return c>=col&&c<col+Size&&r>=row&&r<row+Size;}};
constexpr Area Footprint(int col,int row,int rows){return {std::clamp(col-1,0,9-Size),std::clamp(row-1,0,rows-Size)};}
// Native crater fields persist this clock even after the mushroom is gone.
inline bool Advance(int& remaining,int& countdown){
 if(countdown>0)--countdown;
 if(countdown||remaining<=0)return false;
 --remaining;countdown=Interval;return true;
}
constexpr int FlashAlpha(int countdown){const int age=Interval-countdown;return age>=0&&age<45?75*(45-age)/45:0;}
inline uint32_t EnergyPixel(uint32_t p,int x,int y,int phase){
 const int r=(p>>16)&255,g=(p>>8)&255,b=p&255,light=std::max({r,g,b});
 if(!(p>>24)||light<45)return p;
 const float bend=3*std::sin(y*.15f+phase*.65f);
 const float stripe=std::abs(std::remainder(x+y*.14f+bend,18.f));
 if(stripe>2.8f)return p;
 const float mix=stripe<1.1f?.88f:.52f;
 const int nr=int(r*(1-mix)+(45+light*.18f)*mix),ng=int(g*(1-mix)+(105+light*.50f)*mix),nb=int(b*(1-mix)+(25+light*.14f)*mix);
 return (p&0xff000000u)|(uint32_t(nr)<<16)|(uint32_t(ng)<<8)|uint32_t(nb);
}
}
