#pragma once
#include <algorithm>
#include <cstdint>
#include <array>
// Game ticks, never wall-clock time. The same state drives combat and artwork.
namespace SandboxMemeRules {
inline constexpr int Power=180,Pea=120,Sunflower=121,Wallnut=122,MaxHeat=1000;
inline constexpr int NormalShot=150,BurstShot=18,BurstTime=180,RecoveryTime=240;
inline constexpr int SunInterval=1800,SunRest=900,NutRest=600;
inline constexpr std::array<int,8> LegacyBases{0,1,3,5,7,18,40,23};
constexpr bool LegacyBase(int base){for(int n:LegacyBases)if(n==base)return true;return false;}
inline constexpr auto Bases=[] {std::array<int,48> out{};int n=0;for(int base:LegacyBases)out[n++]=base;for(int base=0;base<48;++base)if(!LegacyBase(base))out[n++]=base;return out;}();
constexpr bool IsPower(int id){return id>=180&&id<=182;}
constexpr bool IsResult(int id){return (id>=120&&id<144)||(id>=300&&id<444&&!LegacyBase((id-300)%48));}
constexpr int PowerOf(int id){return !IsResult(id)?0:id<144?180+(id-120)/8:180+(id-300)/48;}
constexpr int BaseOf(int id){return !IsResult(id)?id:id<144?LegacyBases[(id-120)%8]:(id-300)%48;}
constexpr int Result(int base,int power=Power){
 if(!IsPower(power)||base<0||base>=48)return 0;
 for(int i=0;i<8;++i)if(LegacyBases[i]==base)return 120+(power-180)*8+i;
 return 300+(power-180)*48+base;
}
enum class Kind {Shooter,Producer,Defense,Support,Instant,Chomper,Cannon,Magnet,Torch};
constexpr Kind KindOf(int base){
 switch(base){
 case 1:case 9:case 38:case 41:return Kind::Producer;
 case 3:case 23:case 30:case 36:return Kind::Defense;
 case 16:case 25:case 33:case 37:return Kind::Support;
 case 2:case 4:case 11:case 12:case 14:case 15:case 17:case 19:case 20:case 27:case 35:return Kind::Instant;
 case 6:return Kind::Chomper;
 case 31:case 45:return Kind::Magnet;
 case 22:return Kind::Torch;
 case 47:return Kind::Cannon;
 default:return Kind::Shooter;
 }
}
constexpr int Role(int id){const auto kind=KindOf(BaseOf(id));return kind==Kind::Producer?Sunflower:kind==Kind::Defense||kind==Kind::Support||kind==Kind::Magnet||kind==Kind::Torch?Wallnut:Pea;}
constexpr int Volley(int base){return base==40?4:base==7?2:1;}
enum Phase { Warming, Bursting, Recovering };
enum Event { Nothing=0,Shoot=1,Sun=2,Push=4,ChargedShoot=8,ManySuns=16,Heal=32 };
struct State {int phase=Warming,heat=0,timer=0,delay=45,age=0,pulse=0;};
inline State Initial(int id){State s;if(Role(id)==Sunflower)s.delay=600;if(PowerOf(id)==182)s.delay=Role(id)==Sunflower?2400:400;return s;}
inline void Recover(State& s,int ticks){s.phase=Recovering;s.timer=ticks;s.heat=MaxHeat;s.delay=0;}
inline int Step(State& s,int id,bool target,int damage,bool room){
 s.age=(s.age+1)%1000000;if(s.pulse>0)--s.pulse;
 const int power=PowerOf(id);id=Role(id);
 if(power==181){ // Work steadily; completed actions permanently grow this instance.
  if(id==Wallnut){
   s.heat=std::min(MaxHeat,s.heat+std::max(0,damage));
   if(++s.timer>=600){s.timer=0;s.pulse=20;return Heal;}return Nothing;
  }
  if(s.delay>0)--s.delay;
  if(s.delay||!room||(id==Pea&&!target))return Nothing;
  s.heat=std::min(MaxHeat,s.heat+(id==Sunflower?200:80));s.pulse=10;
  s.delay=id==Sunflower?1800-s.heat*8/10:150-s.heat*60/1000;
  return id==Sunflower?Sun:Shoot;
 }
 if(power==182){ // Deliberate downtime followed by a charged volley / payout.
  if(id==Wallnut){
   s.timer=damage>0?0:(s.timer>=10000?400:s.timer+1);s.heat=std::min(MaxHeat,s.timer*2);
   if(s.timer>=400&&s.timer%100==0){s.pulse=20;return Heal;}return Nothing;
  }
  const int duration=id==Sunflower?2400:400;
  if(s.delay>0)--s.delay;s.heat=(duration-s.delay)*MaxHeat/duration;
  if(s.delay||!room||(id==Pea&&!target))return Nothing;
  s.delay=duration;s.pulse=30;return id==Sunflower?ManySuns:ChargedShoot;
 }
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
constexpr uint32_t PowerPixel(uint32_t pixel,int level,int power){
 const int r=(pixel>>16)&255,g=(pixel>>8)&255,b=pixel&255;
 const int hi=std::max({r,g,b}),lo=std::min({r,g,b});
 if((pixel>>24)==0||hi-lo<12||hi<30)return pixel;
 const int n=std::clamp(level,0,24),red=power==182?lo+(hi-lo)/3:hi;
 const int green=power==181?lo+(hi-lo)*2/3:power==182?lo+(hi-lo)*2/3:lo+(hi-lo)/6;
 const int blue=power==182?hi:lo;
 const auto mix=[&](int a,int z){return (a*(24-n)+z*n+12)/24;};
 return (pixel&0xff000000u)|(uint32_t(mix(r,red))<<16)|(uint32_t(mix(g,green))<<8)|mix(b,blue);
}
constexpr uint32_t WarmPixel(uint32_t pixel,int level){return PowerPixel(pixel,level,180);}
}
