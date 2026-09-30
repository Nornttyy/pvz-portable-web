#pragma once
#include <cmath>
namespace MemeShooterRules {
inline constexpr int PerShot=20,ManualRage=100,MaxRage=200,BurstCount=150,NormalDelay=150,BurstDelay=1;
// Roll once when firing, not every collision/frame. The saved projectile style
// preserves the result across targets, torchwood and save/reload.
inline int NormalStyle(int roll,int variation){return roll==0?10:11+variation%8;}
inline bool CanHit(int style){return style<11||style>18;}
inline bool UsesFreeAim(int style){return style>=1&&style<=9;}
// One gentle breath per 1.6 seconds, driven by saved simulation time so a
// paused board never flashes. No new label or full-body colour overlay.
inline int ReadyGlow(int heat,int phase,int age){
 return phase==0&&heat>=ManualRage?112+int(24*(1-std::cos((age%160)*6.2831853f/160))):0;
}
inline float WobbleStep(int style,int age){
 const float rate=0.070f+(style-1)*0.004f,amplitude=52.0f+(style%3)*7;
 return (style%2?1:-1)*amplitude*(std::sin((age+1)*rate)-std::sin(age*rate));
}
inline float MissStep(int style,int age){
 const int variant=style-10;
 return WobbleStep(variant,age)*1.8f+(variant%2?1.6f:-1.6f);
}
inline float SpreadAngle(int roll){return -0.34f+(roll%1001)*0.68f/1000;}
inline float BurstSpeed(int roll){return 4.6f+(roll%61)*0.05f;}
inline bool OverlapsY(float y,float height,float otherY,float otherHeight){return y+height>=otherY&&y<=otherY+otherHeight;}
}
