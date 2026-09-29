#pragma once
#include "SandboxPlants.h"
#include <vector>
#include <string_view>
class Board;
namespace Sexy {class Graphics;}
namespace MemeAdventure {
struct SavedPlant {unsigned int key=0;SandboxPlants::PowerSave state{};};
struct SavedShot {unsigned int key=0;int percent=100;};
struct Save {int power=0,cooldown=0;std::vector<SavedPlant> plants;std::vector<SavedShot> shots;};
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
Save Capture(Board* board);
void Load(const Save& save);
void LoadShots(const std::vector<SavedShot>& shots);
void Restore(Board* board);
}
