#pragma once
#include <cstddef>
#include <utility>
#include <vector>
class Plant; class Zombie; class Reanimation; class ReanimatorTransform; class ReanimatorTrackInstance;
namespace Sexy {class Image;}
namespace AbstractRigVisuals {
// Per-plant draw scope; shared atlases/cards never inherit the illness colour.
struct NauseaScope {
 std::size_t mark;
 explicit NauseaScope(const Plant*);
 ~NauseaScope();
};
Sexy::Image* NauseatedImage(Reanimation*,Sexy::Image*);
// Scoped draw-only poses. No shared definitions or serialized rig sizes change.
struct Scope {
 std::size_t mark;
 std::vector<std::pair<ReanimatorTrackInstance*,Sexy::Image*>> images;
 std::vector<std::pair<ReanimatorTrackInstance*,int>> groups;
 explicit Scope(const Plant*);
 explicit Scope(Zombie*);
 Scope(Reanimation*,int previewBase);
 ~Scope();
 void Add(Reanimation*,int kind,int pulse=0);
};
void Transform(Reanimation*,int track,ReanimatorTransform&);
}
