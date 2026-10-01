#pragma once
#include <array>
#include <string_view>
namespace ConeBodyRules {
struct Part {std::string_view track;float width,height,cx,cy;};
// Exactly twenty cones. Every visible anatomical track is replaced; these are
// bone-local texture coordinates, not a costume painted over a zombie body.
inline constexpr std::array<Part,20> Parts{{
 {"anim_cone",38,37,29.5f,28.5f},
 {"anim_hair",32,31,14,14},
 {"anim_head1",34,34,25,20},
 {"anim_head2",28,27,16,7.5f},
 {"anim_tongue",18,22,-7,9},
 {"Zombie_neck",27,30,10,10.5f},
 {"Zombie_body",53,55,26.5f,32},
 {"Zombie_tie",32,32,29,38},
 {"anim_innerarm1",26,30,7.5f,15},
 {"anim_innerarm2",24,27,9.5f,12},
 {"anim_innerarm3",24,24,10,11.5f},
 {"Zombie_outerarm_upper",29,36,8.5f,17.5f},
 {"Zombie_outerarm_lower",27,30,9.5f,14},
 {"Zombie_outerarm_hand",29,27,12.5f,13.5f},
 {"Zombie_innerleg_upper",24,28,7.5f,13},
 {"Zombie_innerleg_lower",30,35,16,18},
 {"Zombie_innerleg_foot",28,22,13.5f,8.5f},
 {"Zombie_outerleg_upper",28,36,10.5f,19.5f},
 {"Zombie_outerleg_lower",28,32,12,15},
 {"Zombie_outerleg_foot",40,27,21,10.5f}
}};
constexpr int Index(std::string_view name){for(int i=0;i<int(Parts.size());++i)if(Parts[i].track==name)return i;return -1;}
}
