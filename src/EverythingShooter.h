#pragma once
class Plant;class Zombie;class Projectile;class Reanimation;
namespace Sexy {class Graphics;}
namespace EverythingShooter {
bool IsSlot(int seed);
bool Fire(Plant*,Zombie*);
bool Impact(Projectile*,Zombie*);
bool DrawShot(Sexy::Graphics*,const Projectile*);
void DrawGear(Sexy::Graphics*,Reanimation*);
void DrawPlant(Sexy::Graphics*,const Plant*);
void DrawPreview(Sexy::Graphics*,float x,float y,bool imitater=false);
}
