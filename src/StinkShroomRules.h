#pragma once
#include <algorithm>
namespace StinkShroomRules {
inline constexpr int Id=527, Interval=200, Exposure=2000, StunTicks=50, PushTicks=20;
inline constexpr int Cost=75, Recharge=750, Damage=20;
inline constexpr float PushNormal=40.f,PushGiant=16.f,FleeSpeed=3.2f;
enum Outcome {Normal,Stun,Flee,Push};
// One mutually exclusive roll per struck zombie: 50% / 5% / 10% / 35%.
constexpr Outcome Roll(int value){return value<50?Stun:value<55?Flee:value<65?Push:Normal;}
constexpr bool Adjacent(int ac,int ar,int bc,int br){return (ac!=bc||ar!=br)&&ac>=bc-1&&ac<=bc+1&&ar>=br-1&&ar<=br+1;}
constexpr int ExposureTick(int old,bool nearby){return nearby?std::min(old+1,Exposure):0;}
// Seven work ticks per ten, not interval * 1.3 (which would only slow 23%).
constexpr bool WorkTick(int exposure,unsigned tick){return exposure<Exposure||tick%10>=3;}
}
