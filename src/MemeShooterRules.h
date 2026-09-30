#pragma once
#include <cmath>
namespace MemeShooterRules {
inline constexpr int PerShot=20,ManualRage=100,MaxRage=300,BurstCount=50,NormalDelay=150,BurstDelay=4;
inline float WobbleStep(int style,int age){
 const float rate=0.070f+(style-1)*0.004f,amplitude=52.0f+(style%3)*7;
 return (style%2?1:-1)*amplitude*(std::sin((age+1)*rate)-std::sin(age*rate));
}
inline float SpreadAngle(int index){return -0.26f+((index*17)%BurstCount)*0.52f/(BurstCount-1);}
inline bool OverlapsY(float y,float height,float otherY,float otherHeight){return y+height>=otherY&&y<=otherY+otherHeight;}
}
