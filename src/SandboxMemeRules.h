#pragma once
#include <algorithm>
#include <cstdint>
// Game ticks, never wall-clock time. The same state drives combat and artwork.
namespace SandboxMemeRules {
inline constexpr int Power=180,Pea=120,Sunflower=121,Wallnut=122,MaxHeat=1000;
inline constexpr int NormalShot=150,BurstShot=18,BurstTime=180,RecoveryTime=240;
inline constexpr int SunInterval=1800,SunRest=900,NutRest=600;
constexpr bool IsResult(int id){return id>=Pea&&id<=Wallnut;}
constexpr int Result(int base){return base==0?Pea:base==1?Sunflower:base==3?Wallnut:0;}
enum Phase { Warming, Bursting, Recovering };
enum Event { Nothing=0,Shoot=1,Sun=2,Push=4 };
struct State {int phase=Warming,heat=0,timer=0,delay=45,age=0,pulse=0;};
inline State Initial(int id){State s;if(id==Sunflower)s.delay=600;return s;}
inline void Recover(State& s,int ticks){s.phase=Recovering;s.timer=ticks;s.heat=MaxHeat;s.delay=0;}
inline int Step(State& s,int id,bool target,int damage,bool room){
 ++s.age;if(s.pulse>0)--s.pulse;
 if(s.phase==Recovering){
  const int rest=id==Sunflower?SunRest:id==Wallnut?NutRest:RecoveryTime;
  s.timer=std::max(0,s.timer-1);s.heat=MaxHeat*s.timer/rest;
  if(!s.timer){s.phase=Warming;s.delay=id==Sunflower?SunInterval:45;}
  return Nothing;
 }
 if(id==Wallnut){
  s.heat=std::min(MaxHeat,s.heat+std::max(0,damage)*3);
  if(s.heat==MaxHeat){Recover(s,NutRest);s.pulse=32;return Push;}
  return Nothing;
 }
 if(s.phase==Bursting){
  if(--s.timer<=0){Recover(s,id==Sunflower?SunRest:RecoveryTime);return Nothing;}
  if(s.delay>0)--s.delay;
  if((id==Sunflower||target)&&!s.delay&&room){s.delay=id==Sunflower?55:BurstShot;s.pulse=10;return id==Sunflower?Sun:Shoot;}
  return Nothing;
 }
 if(id==Pea&&!target){s.heat=std::max(0,s.heat-2);s.delay=std::max(25,s.delay-1);return Nothing;}
 if(s.delay>0)--s.delay;
 if(s.delay||!room)return Nothing;
 s.delay=id==Sunflower?SunInterval:NormalShot;s.pulse=10;
 s.heat=std::min(MaxHeat,s.heat+(id==Sunflower?334:200));
 if(s.heat==MaxHeat){s.phase=Bursting;s.timer=BurstTime;s.delay=id==Sunflower?55:BurstShot;}
 return id==Sunflower?Sun:Shoot;
}
// Recolour chromatic pixels only: black outlines, white eyes, alpha and the
// original shading survive. Copies are cached; the source texture is immutable.
constexpr uint32_t WarmPixel(uint32_t pixel,int level){
 const int r=(pixel>>16)&255,g=(pixel>>8)&255,b=pixel&255;
 const int hi=std::max({r,g,b}),lo=std::min({r,g,b});
 if((pixel>>24)==0||hi-lo<12||hi<30)return pixel;
 const int n=std::clamp(level,0,24),red=hi,green=lo+(hi-lo)/6;
 const auto mix=[&](int a,int z){return (a*(24-n)+z*n+12)/24;};
 return (pixel&0xff000000u)|(uint32_t(mix(r,red))<<16)|(uint32_t(mix(g,green))<<8)|mix(b,lo);
}
}
