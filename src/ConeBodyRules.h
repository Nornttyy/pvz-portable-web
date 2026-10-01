#pragma once
#include <array>
#include <string_view>
namespace ConeBodyRules {
struct Part {std::string_view track;float width,height,cx,cy;};
// Exactly twenty cones. Every visible anatomical track is replaced; these are
// bone-local texture coordinates, not a costume painted over a zombie body.
// Chunky, near-native cone proportions: enlarged parts, staggered arm/head
// centres, and raised foot centres so larger bases still meet the same ground.
inline constexpr std::array<Part,20> Parts{{
 {"anim_cone",54,52,29.5f,25.5f},
 {"anim_hair",48,46,11,13},
 {"anim_head1",50,48,27,20},
 {"anim_head2",44,42,13,7.5f},
 {"anim_tongue",34,33,-12,9},
 {"Zombie_neck",40,39,10,10.5f},
 {"Zombie_body",72,70,29.5f,32},
 {"Zombie_tie",48,46,25,38},
 {"anim_innerarm1",40,39,11,15},
 {"anim_innerarm2",38,37,13,12},
 {"anim_innerarm3",36,35,13,11.5f},
 {"Zombie_outerarm_upper",44,43,5.5f,17.5f},
 {"Zombie_outerarm_lower",42,41,5.5f,14},
 {"Zombie_outerarm_hand",44,43,8.5f,13.5f},
 {"Zombie_innerleg_upper",38,37,11.5f,13},
 {"Zombie_innerleg_lower",44,43,19,18},
 {"Zombie_innerleg_foot",42,41,16.5f,-1},
 {"Zombie_outerleg_upper",42,41,7.5f,19.5f},
 {"Zombie_outerleg_lower",42,41,9,15},
 {"Zombie_outerleg_foot",58,56,18,-4}
}};
constexpr int Index(std::string_view name){for(int i=0;i<int(Parts.size());++i)if(Parts[i].track==name)return i;return -1;}
}
