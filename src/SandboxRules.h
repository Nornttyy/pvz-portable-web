#pragma once
#include "SandboxPlants.h"
#include "SandboxZombies.h"
namespace SandboxRules {
inline constexpr int MaxPlants = 180;
inline constexpr int MaxZombies = 160;
constexpr bool ValidMap(int map) { return map == 0 || map == 1; }
constexpr bool ValidCell(int col, int row, bool pool) { return col >= 0 && col < 9 && row >= 0 && row < (pool ? 6 : 5); }
constexpr bool ValidPlant(int type) { return (type >= 0 && type < 48) || SandboxPlants::Find(type); }
constexpr bool ValidSpeed(int speed) { return speed == 1 || speed == 2 || speed == 4; }
// Stacking relaxes occupancy only, not terrain or special target requirements.
struct StackSite {
    bool water=false, lily=false, pot=false, cattail=false, rightLily=false, blocked=false;
};
constexpr bool StackTerrainAllows(int seed,int col,const StackSite& site) {
    if(site.blocked||col<0||col>=9||seed==11||seed==35)return false;
    if(seed==47&&(col==8||(site.water&&!site.rightLily)))return false;
    if(seed==16||seed==19||seed==24||seed==43)return site.water;
    if(site.water){
        if(seed==4||seed==21||seed==33||seed==46)return false;
        return site.lily||(seed==30&&site.cattail);
    }
    return !((seed==21||seed==46)&&site.pot);
}
constexpr bool ValidZombie(int type) {
    return (type >= 0 && type <= 24 && type != 9 && type != 13 && type != 20) || type == 32 || SandboxZombies::Find(type);
}
}
