#pragma once
#include "misc/Rect.h"
#include "misc/SexyMatrix.h"
#include "graphics/Color.h"
class Plant; class Zombie; class Projectile; class Board; class GridItem;
namespace SandboxFactions {
void Reset();
bool Charmed(const Plant*);
bool Charmed(const GridItem*);
void Set(GridItem*,bool);
void Set(Plant*,bool);
void Set(Zombie*,bool);
void Forget(const Plant*);
void Forget(const Projectile*);
bool Enemy(const Plant*,const Plant*);
bool Enemy(const Plant*,const Zombie*);
bool Enemy(const Zombie*,const Plant*);
bool NutContact(Plant*,Zombie*,int padding=35);
int Flags(const Plant*,int);
Sexy::Rect AttackRect(const Plant*,Sexy::Rect);
Plant* Target(Plant*,int row,int weapon=0);
bool HasTarget(Plant*,int row,int weapon=0);
void OnFired(Plant*,Projectile*,Zombie* target=nullptr);
int Direction(const Projectile*);
bool ShotEnemy(const Projectile*,const Plant*);
bool HitPlant(Projectile*,bool lob=false);
void Damage(Plant*,int amount,int freeze=0);
void Area(Plant*,int x,int y,int radius,int rows,int damage,int freeze=0);
void RowDamage(Plant*,int row,int damage,int freeze=0);
bool UpdatePlant(Plant*);
bool Frozen(const Plant*);
void Home(Projectile*);
bool OrbitHit(Plant*,int slot,float x,float y,int damage);
struct ZombieScope { Zombie* previous; explicit ZombieScope(Zombie*); ~ZombieScope(); };
Zombie* ZombieSource();
void SmashZombies(Zombie*,int col,int row,int width=1,int height=1);
struct Scope { Plant* previous; explicit Scope(Plant*); ~Scope(); };
Plant* Source();
struct DrawScope { const Plant* previous; explicit DrawScope(const Plant*); ~DrawScope(); };
void DrawMatrix(Sexy::SexyMatrix3&,Sexy::Color* colour=nullptr);
void DrawOverlay(Sexy::SexyMatrix3&,float x,float y);
}
