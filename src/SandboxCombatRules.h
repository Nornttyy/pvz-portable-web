#pragma once
#include <algorithm>
namespace SandboxCombatRules {
constexpr int PoisonDuration=500, PoisonInterval=100, PoisonDamage=5;
constexpr int EchoDelay=60;
constexpr int GrowthStage(int age){return age>=3500?2:age>=1500?1:0;}
constexpr int GrowthRange(int stage){return stage==2?800:stage==1?640:400;}
constexpr int GrowthDamage(int stage){return stage==2?55:stage==1?38:24;}
constexpr int GrowthRate(int stage){return stage==2?125:stage==1?150:180;}
constexpr bool Ranged(int id){return id==210||id==211;}
constexpr int AimTicks(int id){return id==210?70:100;}
constexpr int RangedRate(int id){return id==210?360:480;}
constexpr int RangedRange(int id){return id==210?450:360;}
constexpr bool EchoPulse(int remaining){return remaining==EchoDelay||remaining==0;}
constexpr bool PoisonPulse(int remaining){return remaining>0&&remaining%PoisonInterval==0;}
// Native secondary shots trigger at EXACTLY 25/50 after decrement. Do not skip or repeat them.
constexpr bool CanHaste(int counter){return counter>1&&counter!=26&&counter!=51;}
constexpr bool CanSlow(int counter){return counter>1&&counter!=25&&counter!=50;}
constexpr int Repair(int health,int maximum){return std::min(maximum,health+25);}
constexpr int SmokeDamage(int damage,int age){return damage>0&&age%700<240?std::max(1,(damage+1)/2):damage;}
constexpr float Speed(int id,bool armor,int burst,bool charged){
 return id==205?1.65f:id==206?0.65f:id==200&&burst>0?1.8f:id==204&&charged?1.35f:1.0f;
}
constexpr float Scale(int id){return id==205?0.83f:id==206?1.04f:1.0f;}
constexpr int CappedTimer(int ticks){return std::clamp(ticks,0,60000);}
}
