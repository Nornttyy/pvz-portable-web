#pragma once
#include <cmath>
namespace SandboxNewPlantRules {
inline constexpr int Walnut=118,Dandelion=119,WalnutHealth=2400;
inline constexpr int SeedCapacity=3,SeedRecharge=160,BurstSpacing=18,BurstRest=110;
// Coordinates relative to the centre of the original 100x100 Wallnut_body.
// Original mouth is 35x49, with its core at (32,24.5). Keep the lip rooted
// several pixels inside the shell, including at maximum shooting recoil.
inline constexpr float NutMouthX=37,NutMouthY=9,NutMouthScale=0.85f;
constexpr bool HasRig(int id){return id==Walnut||id==Dandelion;}
constexpr int DamageStage(int health,int maximum){return maximum<=0?2:health*3<=maximum?2:health*3<=maximum*2?1:0;}
struct Pose {float x,y;};
// This is shared by drawing and bullet birth; never maintain two muzzle offsets.
inline Pose Head(int age,int recoil,int col,int row){
 return {40.0f-recoil*0.25f,28.0f+std::sin((age+col*17+row*31)*0.035f)*1.3f};
}
inline Pose Outlet(int id,Pose head){return {head.x+(id==Walnut?33.0f:24.0f),head.y+(id==Walnut?3.0f:0.0f)};}
}
