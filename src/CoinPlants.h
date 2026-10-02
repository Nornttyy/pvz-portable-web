#pragma once
#include "CoinPlantRules.h"
#include <vector>
class Board;class Plant;class Zombie;class Projectile;class Reanimation;
namespace Sexy {class Graphics;}
namespace CoinPlants {
struct SavedPlant {unsigned key=0;CoinPlantRules::State state;};
using Save=std::vector<SavedPlant>;
bool ShooterSlot(int seed);
bool FlowerSlot(int seed);
void Reset();void Forget(Plant*);void Update(Plant*);
bool Impact(Projectile*,Zombie*);
bool DrawShot(Sexy::Graphics*,const Projectile*);
void DrawGear(Sexy::Graphics*,Reanimation*,bool flower);
void DrawPlant(Sexy::Graphics*,const Plant*);
void DrawOrbit(Sexy::Graphics*,const Plant*,bool front);
void DrawPreview(Sexy::Graphics*,float x,float y,bool flower,bool imitater=false);
Save Capture(Board*);void Load(const Save&);void Restore(Board*);
}
