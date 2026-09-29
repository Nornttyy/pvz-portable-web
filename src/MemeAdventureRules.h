#pragma once
#include "SandboxMemeRules.h"
#include "SandboxUIRules.h"
namespace MemeAdventureRules {
inline constexpr int Cooldown=300; // Native simulation: 100 ticks / second.
inline constexpr SandboxUIRules::Box Slot{704,42,88,38};
constexpr SandboxUIRules::Box Choice(int i){return {616,84+i*39,176,36};}
constexpr int Cost(int power){return power==181?100:75;}
constexpr int Unlock(int power){return power==180?3:power==181?8:13;}
constexpr int RequiredLevel(int power,int base){
 return std::max(Unlock(power),base==1?4:base==3?6:3);
}
constexpr bool Unlocked(int level,bool finished,int power,int base){
 return SandboxMemeRules::IsPower(power)&&SandboxMemeRules::Result(base,power)&&
   (finished||level>=RequiredLevel(power,base));
}
}
