#pragma once
#include <cmath>
#include <cstdint>
namespace MemeShooterRules {
inline constexpr int PerShot=20,MaxRage=300,BurstCount=80,NormalDelay=150,BurstDelay=1;
inline constexpr int FloatingFirst=32,FloatingLast=287,FloatingTurnTicks=36;
inline constexpr bool IsFloating(int style){return style>=FloatingFirst&&style<=FloatingLast;}
// Roll once when firing, not every collision/frame. The saved projectile style
// preserves the result across targets, torchwood and save/reload.
inline int NormalStyle(int roll){return roll==0?10:20;}
inline bool CanHit(int style){return !(style>=11&&style<=18)&&style!=20&&!IsFloating(style);}
inline bool UsesFreeAim(int style){return style>=1&&style<=9;}
// Legacy straight misses keep their saved velocity when loading an old game.
inline float NormalMissAngle(int direction,int roll){return (direction?1.0f:-1.0f)*(0.10f+(roll%1001)*0.32f/1000);}
// Each projectile gets a saved seed, then alternates upper/lower turning points.
// Amplitude changes at every turn without frame-by-frame RNG or accumulated
// drift. Smooth interpolation keeps position/velocity continuous; native age
// plus style reconstruct the exact trajectory after pause, torchwood or reload.
inline uint32_t FloatHash(int style,int turn){
 uint32_t n=uint32_t(style-FloatingFirst+1)*0x9e3779b9u^uint32_t(turn)*0x85ebca6bu;
 n^=n>>16;n*=0x7feb352du;n^=n>>15;n*=0x846ca68bu;return n^(n>>16);
}
inline float FloatingPeak(int style,int turn){
 if(turn==0)return 0;
 const float first=FloatHash(style,0)&1?1.0f:-1.0f;
 return first*(turn%2?1.0f:-1.0f)*(22+FloatHash(style,turn)%55);
}
inline float FloatingOffset(int style,int age){
 const int turn=age/FloatingTurnTicks;const float t=float(age%FloatingTurnTicks)/FloatingTurnTicks;
 const float from=FloatingPeak(style,turn),to=FloatingPeak(style,turn+1);
 return from+(to-from)*(0.5f-0.5f*std::cos(3.14159265f*t));
}
inline float FloatingStep(int style,int age){return FloatingOffset(style,age+1)-FloatingOffset(style,age);}
// Legacy in-flight projectiles retain their old trajectories when loading.
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
