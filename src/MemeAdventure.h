#pragma once
#include "SandboxPlants.h"
#include <vector>
#include <string_view>
class Board;class Zombie;
namespace Sexy {class Graphics;}
namespace MemeAdventure {
// Former power cooldown was unused after roster trim; marker keeps old seed 52
// migration separate from new independent cards, even before one is planted.
inline constexpr int RosterSaveVersion=52300;
constexpr bool LegacyShooterSlot(int version){return version<51900;}
constexpr bool LegacySunflowerSlot(int version){return version<52300;}
struct SavedPlant {unsigned int key=0;SandboxPlants::PowerSave state{};};
struct SavedShot {unsigned int key=0;int percent=100;};
struct SavedZombie {unsigned int key=0;int type=0;};
struct Save {int power=0,cooldown=0;std::vector<SavedPlant> plants;std::vector<SavedShot> shots;std::vector<SavedZombie> zombies;};
bool RosterEnabled();
const MemeCharacters::Definition* Replacement(int seed,int imitater=-1);
std::string_view Translate(std::string_view key,std::string_view original);
void Reset();
bool Visible(Board* board);
void Tick(Board* board);
void Draw(Board* board,Sexy::Graphics* graphics);
bool MouseDown(Board* board,int x,int y,int clicks);
bool Cancel();
void OnPlanted(Plant* plant);
void OnZombieSpawned(Zombie* zombie);
int PickWaveZombie(Board* board,int base,int row,int wave);
Save Capture(Board* board);
void Load(const Save& save);
void LoadShots(const std::vector<SavedShot>& shots);
void LoadZombies(const std::vector<SavedZombie>& zombies);
void Restore(Board* board);
}
