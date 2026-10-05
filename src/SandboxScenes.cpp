#include "SandboxScenes.h"
#include "SandboxSceneRules.h"
#include "Sandbox.h"
#include "SandboxFactions.h"
#include "SandboxPlants.h"
#include "SandboxZombies.h"
#include "LawnApp.h"
#include "Resources.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "Lawn/Projectile.h"
#include "Lawn/GridItem.h"
#include "Lawn/LawnMower.h"
#include "Lawn/System/Music.h"
#include "PvzpLib/Reanimator.h"
#include "PvzpLib/ReanimAtlas.h"
#include "PvzpLib/PvzpCommon.h"
#include "graphics/Graphics.h"
#include <map>
#include <vector>
#include <algorithm>
#include <set>
namespace SandboxScenes {
namespace {std::map<unsigned,int> plantRows,zombieRows;}
void Reset(){plantRows.clear();zombieRows.clear();}
void Preload(){for(const auto& scene:SandboxSceneRules::Scenes)PvzpLoadResources(scene.resources);}
Sexy::Image* Thumbnail(int map){
 switch(map){case 0:return Sexy::IMAGE_BACKGROUND1;case 1:return Sexy::IMAGE_BACKGROUND3;case 2:return Sexy::IMAGE_BACKGROUND2;case 3:return Sexy::IMAGE_BACKGROUND4;case 4:return Sexy::IMAGE_BACKGROUND5;case 5:return Sexy::IMAGE_BACKGROUND6BOSS;default:return nullptr;}
}
bool Switch(Board* b,int map,bool awake){
 if(!gSandboxEnabled||!b||!SandboxSceneRules::Valid(map))return false;
 using namespace SandboxSceneRules;
 const int rows=Rows(map);
 // Reposition the existing instances. Never heal, reassign or reset their combat state.
 struct Position{Plant* plant;int dy;};std::vector<Position> plants;
 for(auto* p:b->mPlants)if(!p->mDead)plants.push_back({p,p->mY-b->GridToPixelY(p->mPlantCol,p->mRow)});
 // Reserve all missing support cells before changing anything. A full pool
 // must leave the old map intact, never strand a retained flower in water.
 std::set<std::pair<int,int>> need,have;
 for(auto entry:plants){auto* p=entry.plant;const auto key=b->mPlants.DataArrayGetID(p);const int r=rows==6&&plantRows.contains(key)?plantRows[key]:std::min(p->mRow,rows-1);
  const int support=Pool(map)&&(r==2||r==3)?16:Roof(map)?33:-1;
  if(support<0)continue;
  const auto cell=std::pair{p->mPlantCol,r};
  if(p->mSeedType==support)have.insert(cell);
  if(p->mSeedType!=16&&p->mSeedType!=33&&p->mSeedType!=19&&p->mSeedType!=24&&p->mSeedType!=43)need.insert(cell);
 }
 for(auto cell:have)need.erase(cell);
 if(b->mPlants.mSize+need.size()>b->mPlants.mMaxSize-8)return false;
 b->mBackground=static_cast<BackgroundType>(Scenes[map].background);b->mLevel=Scenes[map].level;
 b->LoadBackgroundImages();b->mFogOffset=0;b->mFogBlownCountDown=0;b->mSodPosition=0;
 for(int r=0;r<6;++r){
  b->mPlantRow[r]=r>=rows?PLANTROW_DIRT:Pool(map)&&(r==2||r==3)?PLANTROW_POOL:PLANTROW_NORMAL;
  for(int c=0;c<9;++c){b->mGridSquareType[c][r]=r>=rows?GRIDSQUARE_DIRT:Pool(map)&&(r==2||r==3)?GRIDSQUARE_POOL:GRIDSQUARE_GRASS;b->mGridCelFog[c][r]=0;}
 }
 for(auto entry:plants){
  auto* p=entry.plant;const auto key=b->mPlants.DataArrayGetID(p);
  if(p->mRow>=rows){plantRows[key]=p->mRow;p->mRow=rows-1;}
  else if(rows==6&&plantRows.contains(key)){p->mRow=plantRows[key];plantRows.erase(key);}
  p->mX=b->GridToPixelX(p->mPlantCol,p->mRow);p->mY=b->GridToPixelY(p->mPlantCol,p->mRow)+entry.dy;
  p->mRenderOrder=p->CalcRenderOrder();
  if(Plant::IsNocturnal(p->mSeedType))p->SetSleeping(!awake&&!b->StageIsNight());
 }
 // Existing aquatic plants also survive a land switch. Ground plants receive
 // a real support, so normal lily-pad damage/removal continues to work.
 for(auto entry:plants){
  auto* p=entry.plant;const int base=p->mSeedType;
  if(base==16||base==33||base==19||base==24||base==43)continue;
  const int support=b->IsPoolSquare(p->mPlantCol,p->mRow)?16:Roof(map)?33:-1;
  if(support<0)continue;
  bool found=false;for(auto* q:b->mPlants)if(!q->mDead&&q->mPlantCol==p->mPlantCol&&q->mRow==p->mRow&&q->mSeedType==support){found=true;break;}
  if(!found&&b->mPlants.mSize<b->mPlants.mMaxSize-8){Plant::PreloadPlantResources(static_cast<SeedType>(support));auto* pad=b->AddPlant(p->mPlantCol,p->mRow,static_cast<SeedType>(support),SEED_NONE);SandboxFactions::Set(pad,SandboxFactions::Charmed(p));}
 }
 for(auto* z:b->mZombies)if(!z->mDead){
  const auto key=b->mZombies.DataArrayGetID(z);
  if(z->mRow>=rows){zombieRows[key]=z->mRow;z->mRow=rows-1;}
  else if(rows==6&&zombieRows.contains(key)){z->mRow=zombieRows[key];zombieRows.erase(key);}
  if(z->mZombieType!=ZOMBIE_BOSS){z->mPosY=z->GetPosYBasedOnRow(z->mRow);z->mY=int(z->mPosY);z->mRenderOrder=Board::MakeRenderOrder(RENDER_LAYER_ZOMBIE,z->mRow,0);UpdateSwimmer(z);z->UpdateReanim();}
 }
 // Discard old trajectories; relocate persistent terrain items with the grid.
 for(auto* shot:b->mProjectiles)if(!shot->mDead)shot->Die();
 for(auto* item:b->mGridItems)if(!item->mDead&&item->mGridX>=0&&item->mGridX<9&&item->mGridY>=0){item->mGridY=std::min(item->mGridY,rows-1);item->mPosX=b->GridToPixelX(item->mGridX,item->mGridY);item->mPosY=b->GridToPixelY(item->mGridX,item->mGridY);}
 for(auto* mower:b->mLawnMowers)if(!mower->mDead)mower->Die();
 b->mApp->mMusic->StartGameMusic();b->MarkAllDirty();return true;
}
bool UpdateSwimmer(Zombie* z){
 if(!gSandboxEnabled||!z||!z->mBoard||z->mZombieType==ZOMBIE_BOSS)return false;
 const bool water=z->mBoard->StageHasPool()&&(z->mRow==2||z->mRow==3)&&z->mPosX>-40&&z->mPosX<760&&!z->IsFlying();
 // Direct placement bypasses the native x=700 entry trigger. Enter the
 // appropriate native swimming pose once, without restarting dive attacks.
 if(z->mZombieType==ZOMBIE_SNORKEL||z->mZombieType==ZOMBIE_DOLPHIN_RIDER){
  if(z->IsDeadOrDying())return true;
  const bool snorkel=z->mZombieType==ZOMBIE_SNORKEL;
  const auto phase=z->mZombiePhase;
  if(water&&!z->mInPool&&(phase==PHASE_SNORKEL_WALKING||phase==PHASE_DOLPHIN_WALKING||phase==PHASE_DOLPHIN_WALKING_WITHOUT_DOLPHIN)){
   z->mInPool=true;z->mZombieHeight=HEIGHT_ZOMBIE_NORMAL;z->mAltitude=0;
   const bool riding=!snorkel&&phase==PHASE_DOLPHIN_WALKING;
   z->mZombiePhase=snorkel?PHASE_SNORKEL_WALKING_IN_POOL:riding?PHASE_DOLPHIN_RIDING:PHASE_DOLPHIN_WALKING_IN_POOL;
   z->PlayZombieReanim(riding?"anim_ride":"anim_swim",REANIM_LOOP_FULL_LAST_FRAME,0,12.f);
  }else if(!water&&(z->mInPool||phase==PHASE_SNORKEL_INTO_POOL||phase==PHASE_DOLPHIN_INTO_POOL)){
   const bool dolphin=phase==PHASE_DOLPHIN_RIDING||phase==PHASE_DOLPHIN_INTO_POOL||phase==PHASE_DOLPHIN_WALKING;
   z->mInPool=false;z->mZombieHeight=HEIGHT_ZOMBIE_NORMAL;z->mAltitude=0;z->mIsEating=false;
   z->mZombiePhase=snorkel?PHASE_SNORKEL_WALKING:dolphin?PHASE_DOLPHIN_WALKING:PHASE_DOLPHIN_WALKING_WITHOUT_DOLPHIN;
   z->PlayZombieReanim(dolphin?"anim_walkdolphin":"anim_walk",REANIM_LOOP,0,12.f);z->PickRandomSpeed();
  }
  return true;
 }
 const bool changed=z->mInPool!=water;z->mInPool=water;
 if(z->mZombieHeight==HEIGHT_IN_TO_POOL||z->mZombieHeight==HEIGHT_OUT_OF_POOL)z->mZombieHeight=HEIGHT_ZOMBIE_NORMAL;
 if(z->mZombieHeight==HEIGHT_ZOMBIE_NORMAL&&!SandboxZombies::IsDodging(z))z->mAltitude=water?-40*z->mScaleZombie:0;
 if(changed){if(auto* anim=z->mApp->ReanimationTryToGet(z->mBodyReanimID);anim&&anim->TrackExists("Zombie_duckytube"))z->ReanimShowPrefix("Zombie_duckytube",water?RENDER_GROUP_NORMAL:RENDER_GROUP_HIDDEN);}
 return true;
}
void MoveFollowers(Zombie* z,float dx){
 if(z->mZombieType!=ZOMBIE_BOBSLED)return;
 for(auto id:z->mFollowerZombieID)if(auto* follower=z->mBoard->ZombieTryToGet(id)){
  follower->mPosX+=dx;follower->mX=int(follower->mPosX);UpdateSwimmer(follower);follower->UpdateReanim();
 }
}
void DrawSwimRing(Sexy::Graphics* g,Zombie* z){
 if(!gSandboxEnabled||!z->mInPool||z->IsFlying()||z->mZombieType==ZOMBIE_BOSS||z->mZombieType==ZOMBIE_SNORKEL||z->mZombieType==ZOMBIE_DOLPHIN_RIDER)return;
 auto* body=z->mApp->ReanimationTryToGet(z->mBodyReanimID);
 if(body&&body->TrackExists("Zombie_duckytube")&&!SandboxZombies::IsConeWrap(z))return;
 // Reuse the original duck ring image, sized to the actual collision body.
 static Sexy::Image* image=nullptr;
 if(!image){Reanimation ring;ring.ReanimationInitializeType(0,0,REANIM_ZOMBIE);ring.SetFramesForLayer("anim_walk");if(!ring.TrackExists("Zombie_duckytube"))return;ReanimatorTransform pose;ring.GetCurrentTransform(ring.FindTrackIndex("Zombie_duckytube"),&pose);image=pose.mImage;if(auto* atlas=ring.mDefinition->mReanimAtlas)if(auto* encoded=atlas->GetEncodedReanimAtlas(image))image=encoded->mOriginalImage;}
 if(!image)return;
 ZombieDrawPosition draw;z->GetDrawPos(draw);const float w=std::clamp(z->mZombieRect.mWidth*1.25f,68.f,156.f)*z->mScaleZombie;
 const float h=w*image->mHeight/image->mWidth;
 g->DrawImage(image,int(z->mZombieRect.mX+z->mZombieRect.mWidth*.5f-w*.5f),int(draw.mBodyY+90*z->mScaleZombie-h*.5f),int(w),int(h));
}
}
