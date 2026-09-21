// State-only engine doubles. Tests compile UNMODIFIED production combat modules;
// real native compilation separately checks ABI/types and drawing code.
#pragma once
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <algorithm>
#include <cstdint>
enum SeedType {SEED_PEASHOOTER=0,SEED_NONE=-1};
enum ZombieType {ZOMBIE_NORMAL=0,ZOMBIE_IMP=24,ZOMBIE_GARGANTUAR=23,ZOMBIE_REDEYE_GARGANTUAR=32,ZOMBIE_ZAMBONI=12};
enum ZombieID : unsigned { ZOMBIEID_NULL=0 };
enum ReanimationType {REANIM_ZOMBIE,REANIM_FLAG};
enum ProjectileType {PROJECTILE_PEA,PROJECTILE_SNOWPEA,PROJECTILE_FIREBALL};
enum ProjectileMotion {MOTION_STRAIGHT,MOTION_STAR,MOTION_HOMING,MOTION_THREEPEATER};
enum PlantWeapon {WEAPON_PRIMARY};
enum PlantSubClass {SUBCLASS_NORMAL,SUBCLASS_SHOOTER};
constexpr int RENDER_GROUP_HIDDEN=-1,DS_ALIGN_CENTER=0;
namespace Sexy {
struct Color{int mAlpha;Color(int=0,int=0,int=0,int a=255):mAlpha(a){}};
struct SexyTransform2D{float m00=1,m01=0,m02=0,m10=0,m11=1,m12=0;void LoadIdentity(){*this={};}};
struct Rect{int mX,mY,mWidth,mHeight;Rect(int x=0,int y=0,int w=0,int h=0):mX(x),mY(y),mWidth(w),mHeight(h){}};
struct Image{int mWidth=80,mHeight=80;std::string path;virtual~Image()=default;};
struct MemoryImage:Image{std::vector<uint32_t> bits=std::vector<uint32_t>(6400,0xffffffff);uint32_t* GetBits(){return bits.data();}};
struct GLImage:MemoryImage{};
struct Graphics{
 float mTransX=0,mTransY=0;Rect mClipRect;int mDrawMode=0;
 Graphics(MemoryImage*){};Graphics(const Graphics&)=default;
 void SetLinearBlend(bool){};void SetColor(Color){};void DrawLine(int,int,int,int){};void FillRect(int,int,int,int){};void SetClipRect(int,int,int,int){};
 void ClipRect(int,int,int,int){};
 void DrawImage(Image*,Rect,Rect){};
};
inline Image* IMAGE_SEEDS=nullptr;inline int FONT_BRIANNETOD12=0;
}
struct Track{const char* mName="";};
struct TrackGroup{int count=0;Track* tracks=nullptr;};
struct Definition{TrackGroup mTracks;};
struct TrackInstance{Sexy::Image* mImageOverride=nullptr;int mRenderGroup=0;};
struct ReanimatorTransform{float mFrame=0,mAlpha=1;};
struct Reanimation{
 Definition def;Definition* mDefinition=&def;TrackInstance* mTrackInstances=nullptr;float mAnimTime=0;
 std::string track;Sexy::SexyTransform2D matrix;ReanimatorTransform pose;
 void ReanimationInitializeType(int,int,ReanimationType){};bool TrackExists(const char* name){return track==name;}
 void SetFramesForLayer(const char*){};void Draw(Sexy::Graphics*){};void SetImageOverride(const char*,Sexy::Image*){};Reanimation* FindSubReanim(ReanimationType){return nullptr;}
 int mFrameBasePose=0;Sexy::SexyTransform2D mOverlayMatrix;int FindTrackIndex(const char*){return 0;}void GetAttachmentOverlayMatrix(int,Sexy::SexyTransform2D&){};
 void GetTrackMatrix(int,Sexy::SexyTransform2D& out){out=matrix;}void GetCurrentTransform(int,ReanimatorTransform* out){*out=pose;}
};
struct ReanimatorCache{std::unique_ptr<Sexy::MemoryImage> MakeBlankMemoryImage(int,int){return std::make_unique<Sexy::MemoryImage>();}};
struct LawnApp{ReanimatorCache cache;ReanimatorCache* mReanimatorCache=&cache;std::map<int,Reanimation*> reanims;Reanimation* ReanimationTryToGet(int id){return reanims.contains(id)?reanims.at(id):nullptr;}Sexy::GLImage* GetImage(std::string file){auto* im=new Sexy::GLImage;im->path=file;return im;}};
extern LawnApp* gLawnApp;
class Board;
class Zombie{
public:
 static constexpr int ZOMBIE_WAVE_DEBUG=-1;
 Board* mBoard=nullptr;ZombieType mZombieType=ZOMBIE_NORMAL;ZombieID id=ZOMBIEID_NULL;
 float mPosX=0,mPosY=0,mScaleZombie=1;int mX=0,mY=0,mRow=0,mBodyReanimID=0,mBodyHealth=1000,mBodyMaxHealth=1000,mHelmHealth=0,mHelmMaxHealth=0;
 bool mDead=false,mMindControlled=false,mHasHead=true;int chill=0;
 int mSpecialHeadReanimID=0;
 bool IsDeadOrDying(){return mDead||mBodyHealth<=0;};bool EffectedByDamage(unsigned){return !IsDeadOrDying();}
 void TakeDamage(int n,unsigned){int armor=std::min(n,mHelmHealth);mHelmHealth-=armor;mBodyHealth-=n-armor;}
 void UpdateReanim(){};void RemoveColdEffects(){chill=0;}
 static void PreloadZombieResources(ZombieType){};static void SetupReanimLayers(Reanimation*,ZombieType){}
};
class Plant{
public:
 Board* mBoard=nullptr;SeedType mSeedType=SEED_PEASHOOTER;
 int mX=0,mY=0,mRow=0,mPlantCol=0,mPlantHealth=300,mPlantMaxHealth=300,mLaunchRate=150,mLaunchCounter=100,mBlinkCountdown=0,mShootingCounter=0;
 int mBodyReanimID=0,mHeadReanimID=0,mHeadReanimID2=0,mHeadReanimID3=0;
 bool mDead=false,mIsAsleep=false;
 int GetDamageRangeFlags(PlantWeapon){return 0;};Zombie* FindTargetZombie(int row,PlantWeapon);
};
class Projectile{
public:
 Board* mBoard=nullptr;bool mDead=false;ProjectileMotion mMotionType=MOTION_STRAIGHT;ZombieID mTargetZombieID=ZOMBIEID_NULL;
 float mPosX=0,mPosY=0,mPosZ=0,mVelX=3.3,mVelY=0,mShadowY=0;
 int mX=0,mY=0,mRow=0,mRenderOrder=0,mDamageRangeFlags=0;
 ProjectileType mProjectileType=PROJECTILE_PEA;
 int GetDamageFlags(Zombie*){return 0;}
};
template<class T>struct Array{
 std::vector<T*> values;int mSize=0,mMaxSize=256;
 auto begin(){return values.begin();}auto end(){return values.end();}
 void add(T* t){values.push_back(t);++mSize;}
};
class Board{
 unsigned nextID=1;
public:
 bool mPaused=false,pool=false;int mMainCounter=0;
 Array<Plant> mPlants;Array<Zombie> mZombies;Array<Projectile> mProjectiles;
 std::vector<std::unique_ptr<Plant>> ownedPlants;std::vector<std::unique_ptr<Zombie>> ownedZombies;std::vector<std::unique_ptr<Projectile>> ownedShots;
 bool StageHasPool(){return pool;}
 ZombieID ZombieGetID(Zombie* z){return z?z->id:ZOMBIEID_NULL;}
 Zombie* ZombieTryToGet(ZombieID id){for(auto* z:mZombies)if(z->id==id)return z;return nullptr;}
 Zombie* AddZombieInRow(ZombieType type,int row,int){
  auto p=std::make_unique<Zombie>();auto* z=p.get();z->mBoard=this;z->id=ZombieID(nextID++);z->mZombieType=type;z->mRow=row;z->mPosY=row*100;ownedZombies.push_back(std::move(p));mZombies.add(z);return z;
 }
 Projectile* AddProjectile(float x,float y,int order,int row,ProjectileType){
  auto p=std::make_unique<Projectile>();auto* s=p.get();s->mBoard=this;s->mPosX=x;s->mPosY=y;s->mRenderOrder=order;s->mRow=row;ownedShots.push_back(std::move(p));mProjectiles.add(s);return s;
 }
 Plant* plant(int col,int row){auto p=std::make_unique<Plant>();auto* a=p.get();a->mBoard=this;a->mPlantCol=col;a->mRow=row;a->mX=col*80;a->mY=row*100;ownedPlants.push_back(std::move(p));mPlants.add(a);return a;}
};
inline Zombie* Plant::FindTargetZombie(int row,PlantWeapon){for(auto* z:mBoard->mZombies)if(!z->IsDeadOrDying()&&z->mRow==row&&!z->mMindControlled&&z->mPosX>=mX)return z;return nullptr;}
struct PlantDefinition{ReanimationType mReanimationType=REANIM_ZOMBIE;PlantSubClass mSubClass=SUBCLASS_SHOOTER;};
inline PlantDefinition GetPlantDefinition(SeedType type){return {REANIM_ZOMBIE,int(type)==1||int(type)==3?SUBCLASS_NORMAL:SUBCLASS_SHOOTER};}
inline void DrawSeedPacket(Sexy::Graphics*,int,int,SeedType,SeedType,int,int,bool,bool){}
inline void PvzpDrawImageCelScaledF(Sexy::Graphics*,Sexy::Image*,int,int,int,int,int,int){}
inline void PvzpDrawString(Sexy::Graphics*,const char*,int,int,int,Sexy::Color,int){}
struct Blit{std::string path;Sexy::SexyTransform2D matrix;int alpha;};
inline std::vector<Blit> testBlits;
inline void PvzpBltMatrix(Sexy::Graphics*,Sexy::Image* im,const Sexy::SexyTransform2D& mat,Sexy::Rect,Sexy::Color color,int,Sexy::Rect){testBlits.push_back({im->path,mat,color.mAlpha});}
