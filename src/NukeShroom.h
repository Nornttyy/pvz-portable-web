#pragma once
class Plant;class Board;class GridItem;
namespace Sexy {class Graphics;}
namespace NukeShroom {
bool Detonate(Plant*);
bool IsCrater(const GridItem*);
void UpdateCrater(GridItem*);
bool DrawCrater(Sexy::Graphics*,GridItem*);
void DrawScreen(Sexy::Graphics*,Board*);
}
