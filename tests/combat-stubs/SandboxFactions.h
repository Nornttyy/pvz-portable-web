#pragma once
// This fixture tests the legacy combat implementation. Actual faction combat
// is exercised by qa-sandbox-factions.mjs against the compiled WASM engine.
#include "Engine.h"
namespace SandboxFactions {
struct Scope {explicit Scope(Plant*){}};
inline bool Frozen(const Plant*){return false;}
inline bool NutContact(Plant* p,Zombie* z,int padding=35){return z->mPosX>=p->mX-65&&z->mPosX<p->mX+padding;}
inline bool Enemy(const Plant*,const Zombie* z){return !z->mMindControlled;}
inline bool Enemy(const Zombie* z,const Plant*){return !z->mMindControlled;}
inline Plant* Target(Plant*,int,int=0){return nullptr;}
inline bool HasTarget(Plant* p,int row,int weapon=0){return p->FindTargetZombie(row,static_cast<PlantWeapon>(weapon));}
inline void OnFired(Plant*,Projectile*,Zombie*){}
inline void DrawOverlay(Sexy::SexyTransform2D&,float,float){}
}
