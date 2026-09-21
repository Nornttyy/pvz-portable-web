#pragma once
// Measured in the native 800 x 600 game canvas. CSS scaling never changes these.
namespace SandboxVisualRules {
inline constexpr float PeaCenter=12.0f;
inline constexpr float GroundX=40.0f,GroundY=65.0f;
struct ShotArt {const char* sprite;const char* hit0;const char* hit1;float w,h,coreX,coreY,impact;};
inline constexpr ShotArt Shots[]{
 {"fire","fire-hit-0","fire-hit-1",30,22,20,11,38},
 {"ice","ice-hit-0","ice-hit-1",23,20,14,10,32},
 {"electric","electric-hit-0","electric-hit-1",25,23,16,12,34},
 {"tiny","tiny-hit-0","tiny-hit-1",10,10,5,5,19},
 {"heavy","heavy-hit-0","heavy-hit-1",24,24,12,12,40},
 {"scatter","scatter-hit-0","scatter-hit-1",13,13,6.5f,6.5f,24},
 {"seeker","seeker-hit-0","seeker-hit-1",25,18,17,9,29},
 {"acid","acid-hit-0","acid-hit-1",23,23,14,10,33}
};
constexpr int HeadForRow(int plantRow,int shotRow){return shotRow>plantRow?1:shotRow<plantRow?3:2;}
constexpr int ArtIndex(int id,bool fire,bool ice){return fire?0:ice?1:id>=112&&id<=117?id-110:3;}
}
