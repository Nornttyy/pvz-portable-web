#pragma once
class Board;class Zombie;
namespace Sexy {class Graphics;class Image;}
namespace SandboxScenes {
void Reset();
void Preload();
Sexy::Image* Thumbnail(int map);
bool Switch(Board*,int map,bool awake);
bool UpdateSwimmer(Zombie*);
void MoveFollowers(Zombie*,float dx);
void DrawSwimRing(Sexy::Graphics*,Zombie*);
}
