#pragma once
// Measured in the native 800 x 600 game canvas. CSS scaling never changes these.
namespace SandboxVisualRules {
inline constexpr float PeaCenter=12.0f;
inline constexpr float GroundX=40.0f,GroundY=65.0f;
constexpr int HeadForRow(int plantRow,int shotRow){return shotRow>plantRow?1:shotRow<plantRow?3:2;}
// Pixel hit radii are deliberately independent from the art canvas.  This
// avoids the oversized projectile collision bug when a future rig has wider
// transparent margins than another one.
constexpr int TechShotRadius(int id){
 return id==104?17:id==106?15:id==107?13:id==105?14:12;
}
}
