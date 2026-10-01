#include "NukeShroom.h"
#include "NukeShroomRules.h"
#include "MemeCharacters.h"
#include "LawnApp.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "Lawn/GridItem.h"
#include "Resources.h"
#include "PvzpLib/PvzpParticle.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include <algorithm>
namespace NukeShroom {
namespace {
void Pulse(GridItem* crater){
 auto* board=crater->mBoard;
 // Never hit roadside seed-selection previews or friendly hypnotized zombies.
 for(auto* z:board->mZombies)if(!z->mDead&&z->IsOnBoard()&&!z->mMindControlled&&!z->IsDeadOrDying()&&z->EffectedByDamage(127))z->ApplyBurn();
 for(auto* item:board->mGridItems)if(!item->mDead&&item->mGridItemType==GRIDITEM_LADDER)item->GridItemDie();
 board->mApp->PlaySample(Sexy::SOUND_DOOMSHROOM);
 if(auto* cloud=board->mApp->AddPvzpParticle(crater->mPosX,crater->mPosY,int(RENDER_LAYER_TOP),PARTICLE_DOOM)){
  cloud->OverrideColor(nullptr,Sexy::Color(85,235,65));
  // Replace the original red/white flash with a mild green screen wash.
  cloud->OverrideColor("DoomFlash",Sexy::Color(0,0,0,0));
  cloud->OverrideColor("DoomWord",Sexy::Color(0,0,0,0));
 }
 board->ShakeBoard(3,-4);
}
}
bool IsCrater(const GridItem* item){return item&&!item->mDead&&item->mGridItemType==GRIDITEM_CRATER&&int(item->mGridItemState)>=NukeShroomRules::CraterMarker&&int(item->mGridItemState)<NukeShroomRules::CraterMarker+9;}
bool Detonate(Plant* plant){
 if(MemeCharacters::Type(plant)!=NukeShroomRules::Id||!plant->mBoard)return false;
 auto* board=plant->mBoard;
 if(board->mGridItems.mSize>board->mGridItems.mMaxSize-9)return false;
 const auto area=NukeShroomRules::Footprint(plant->mPlantCol,plant->mRow,board->StageHasPool()?6:5);
 const float x=plant->mX+40,y=plant->mY+40;
 // Clear exactly nine cells, including pads/pots/shells and either half of a cannon.
 for(auto* p:board->mPlants)if(!p->mDead&&(area.Contains(p->mPlantCol,p->mRow)||(p->mSeedType==SEED_COBCANNON&&area.Contains(p->mPlantCol+1,p->mRow))))p->Die();
 for(auto* item:board->mGridItems)if(!item->mDead&&area.Contains(item->mGridX,item->mGridY)&&item->mGridItemType==GRIDITEM_GRAVESTONE)item->GridItemDie();
 GridItem* primary=nullptr;
 for(int dy=0;dy<3;++dy)for(int dx=0;dx<3;++dx){
  auto* crater=board->AddACrater(area.col+dx,area.row+dy);
  crater->mGridItemState=GridItemState(NukeShroomRules::CraterMarker+dy*3+dx);
  crater->mGridItemCounter=NukeShroomRules::CraterLife;
  crater->mPosX=x;crater->mPosY=y;crater->mSunCount=0;crater->mTransparentCounter=0;
  if(dx==0&&dy==0){primary=crater;crater->mSunCount=NukeShroomRules::Pulses-1;crater->mTransparentCounter=NukeShroomRules::Interval;}
 }
 Pulse(primary);return true;
}
void UpdateCrater(GridItem* item){
 if(!IsCrater(item)||int(item->mGridItemState)!=NukeShroomRules::CraterMarker||item->mBoard->mPaused)return;
 if(NukeShroomRules::Advance(item->mSunCount,item->mTransparentCounter))Pulse(item);
}
bool DrawCrater(Sexy::Graphics* g,GridItem* item){
 if(!IsCrater(item))return false;
 auto* b=item->mBoard;const int part=int(item->mGridItemState)-NukeShroomRules::CraterMarker;
 const int col=item->mGridX-part%3,row=item->mGridY-part/3;
 const bool pool=b->StageHasPool(),water=b->IsPoolSquare(item->mGridX,item->mGridY);
 const int pitch=(pool||b->StageHasRoof())?85:100;
 int start=row,end=row+3;
 // A shoreline must not slice an enlarged crater in half. Use complete native
 // land/water rims for each contiguous terrain band, covering the same nine cells.
 if(pool){if(water){start=std::max(start,2);end=std::min(end,4);}else if(item->mGridY<2)end=std::min(end,2);else start=std::max(start,4);}
 auto top=[&](int r){if(pool){if(r==2)return 279;if(r==3)return 362;if(r==4)return 445;if(r==6)return 590;}
  return b->GridToPixelY(item->mGridX,0)+r*pitch;};
 const float x=b->GridToPixelX(col,row)-4.f,y=top(start)+8.f;
 const bool fading=item->mGridItemCounter<9000;
 auto* image=fading?Sexy::IMAGE_CRATER_FADING:Sexy::IMAGE_CRATER;int cel=b->StageIsNight()?1:0;
 if(water){image=b->StageIsNight()?Sexy::IMAGE_CRATER_WATER_NIGHT:Sexy::IMAGE_CRATER_WATER_DAY;cel=fading?1:0;}
 else if(b->StageHasRoof()){image=item->mGridX<5?Sexy::IMAGE_CRATER_ROOF_LEFT:Sexy::IMAGE_CRATER_ROOF_CENTER;cel=fading?1:0;}
 Sexy::Graphics draw(*g);
 // The native per-row render order and shovel still own each of the nine cells.
 draw.ClipRect(b->GridToPixelX(item->mGridX,item->mGridY)-4,top(item->mGridY),part%3==2?88:80,top(item->mGridY+1)-top(item->mGridY));
 draw.SetColor(Sexy::Color(220,242,213,std::min(255,item->mGridItemCounter*255/25)));draw.SetColorizeImages(true);
 PvzpDrawImageCelScaledF(&draw,image,x,y,cel,0,248.f/image->GetCelWidth(),(top(end)-top(start)-20.f)/image->GetCelHeight());return true;
}
void DrawScreen(Sexy::Graphics* g,Board* b){
 int alpha=0;for(auto* item:b->mGridItems)if(IsCrater(item)&&int(item->mGridItemState)==NukeShroomRules::CraterMarker)alpha=std::max(alpha,NukeShroomRules::FlashAlpha(item->mTransparentCounter));
 if(!alpha)return;Sexy::Graphics wash(*g);wash.SetColor(Sexy::Color(80,220,45,alpha));wash.FillRect(-b->mX,-b->mY,b->mApp->mWidth,b->mApp->mHeight);
}
}
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
// Read-only diagnostics used by browser regression tests; no save/game mutations.
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_nuke_data(int index,int field){
 auto* board=gLawnApp?gLawnApp->mBoard:nullptr;if(!board)return -1;
 int count=0;for(auto* item:board->mGridItems)if(NukeShroom::IsCrater(item)){
  if(count++==index)switch(field){case 0:return item->mGridX;case 1:return item->mGridY;case 2:return item->mGridItemCounter;case 3:return item->mSunCount;case 4:return item->mTransparentCounter;case 5:return int(item->mGridItemState)-NukeShroomRules::CraterMarker;}
 }
 return index<0?count:-1;
}
#endif
