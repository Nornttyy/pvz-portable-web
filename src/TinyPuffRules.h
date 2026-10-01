#pragma once
#include <array>
namespace TinyPuffRules {
inline constexpr int Limit=5, Recharge=200, Damage=10;
inline constexpr float Scale=1.0f/3.0f, AnchorX=40, AnchorY=65;
struct Pose {float x,y,lean,cap;};
// First the centre, then two below, then one on each side. Stable slots:
// removing a mushroom never moves the four survivors around the tile.
inline constexpr std::array<Pose,Limit> Poses{{
 {40,55,0,0}, {28,74,-.20f,-.10f}, {52,74,.18f,.09f},
 {16,55,-.32f,.13f}, {64,55,.27f,-.12f}
}};
constexpr Pose At(int slot){return Poses[slot>=0&&slot<Limit?slot:0];}
}
