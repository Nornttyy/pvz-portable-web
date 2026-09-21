#pragma once
#include <algorithm>
namespace SandboxCombatRules {
constexpr int PoisonDuration=500, PoisonInterval=100, PoisonDamage=5;
constexpr int SpringCooldown=350, EchoDelay=60, NetRange=3;
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
constexpr bool Connected(int c1,int r1,int c2,int r2){return c1==c2&&r1<r2&&r2-r1<=NetRange;}
constexpr int CappedTimer(int ticks){return std::clamp(ticks,0,60000);}
}
