#pragma once
#include <algorithm>
#include <array>
#include "SandboxMemeRules.h"
// Native ingredients or one meme power. No recursive chains
// or implicit Base() matching: a fused plant is never silently treated as a pea.
namespace SandboxFusion {
struct Recipe { int first,second,result; };
inline constexpr std::array<Recipe,4> Recipes{{
 {0,3,118},{0,180,120},{1,180,121},{3,180,122}
}};
constexpr int Result(int first,int second) {
 for(const auto& recipe:Recipes)
  if((recipe.first==first&&recipe.second==second)||(recipe.first==second&&recipe.second==first))return recipe.result;
 return 0;
}
constexpr bool SupportLayer(int seed){return seed==16||seed==33||seed==30||seed==35;}
constexpr int InheritedHealth(int health,int maximum,int resultMaximum){
 if(maximum<=0||resultMaximum<=0)return 1;
 const auto scaled=static_cast<long long>(std::clamp(health,1,maximum))*resultMaximum/maximum;
 return std::clamp(static_cast<int>(scaled),1,resultMaximum);
}
}
