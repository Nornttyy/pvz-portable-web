#pragma once
#include "SandboxPlants.h"
namespace SandboxRules {
inline constexpr int MaxPlants = 180;
inline constexpr int MaxZombies = 160;
constexpr bool ValidMap(int map) { return map == 0 || map == 1; }
constexpr bool ValidCell(int col, int row, bool pool) { return col >= 0 && col < 9 && row >= 0 && row < (pool ? 6 : 5); }
constexpr bool ValidPlant(int type) { return (type >= 0 && type < 48) || SandboxPlants::Find(type); }
constexpr bool ValidSpeed(int speed) { return speed == 1 || speed == 2 || speed == 4; }
constexpr bool ValidZombie(int type) {
    return (type >= 0 && type <= 24 && type != 9 && type != 13 && type != 20) || type == 32;
}
}
