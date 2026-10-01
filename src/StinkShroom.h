#pragma once
#include <vector>
class Board;class Plant;class Zombie;
namespace Sexy {class Graphics;}
namespace StinkShroom {
struct PlantSave {unsigned key=0;int exposure=0;};
struct ZombieSave {unsigned key=0;int stun=0,push=0,flee=0,stepMilli=0;};
struct Save {std::vector<PlantSave> plants;std::vector<ZombieSave> zombies;};
void Reset();void Forget(Plant*);void Forget(Zombie*);
void UpdatePlant(Plant*);bool Affected(const Plant*);bool WorkTick(const Plant*);
void Hit(Plant*,Zombie*);bool Stunned(const Zombie*);bool Fleeing(const Zombie*);bool Controls(const Zombie*);
bool UpdateZombie(Zombie*);
void DrawEffects(Sexy::Graphics*,Board*,int row);
Save Capture(Board*);void Load(const Save&);void Restore(Board*);
}
