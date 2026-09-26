#pragma once
#include <algorithm>
namespace SandboxCombatRules {
// The technology roster deliberately has one ranged enemy only.  Keeping the
// timing here makes its stop / aim / fire rhythm testable without rendering.
constexpr bool Ranged(int id){return id==205;}
constexpr int AimTicks(int){return 54;}
constexpr int RangedRate(int){return 300;}
constexpr int RangedRange(int){return 420;}
constexpr int Repair(int health,int maximum){return std::min(maximum,health+25);}
constexpr float Speed(int id,bool armor,int burst,bool){
 return id==202?(burst>0?1.9f:0.78f):id==206?1.15f:id==201&&armor?0.82f:id==208?0.84f:1.0f;
}
constexpr float Scale(int id){return id==206?0.93f:id==208?1.05f:id==202?0.96f:1.0f;}
constexpr int CappedTimer(int ticks){return std::clamp(ticks,0,60000);}
}
