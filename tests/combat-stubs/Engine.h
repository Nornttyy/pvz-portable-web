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
constexpr int PHASE_ZOMBIE_NORMAL=0,HEIGHT_ZOMBIE_NORMAL=0,HEIGHT_FALLING=1;
constexpr int STATE_READY=1,STATE_CHOMPER_DIGESTING=13,STATE_SCAREDYSHROOM_SCARED=21,STATE_CACTUS_LOW=30,STATE_CACTUS_HIGH=32;
constexpr int PHASE_NEWSPAPER_MAD=2,PHASE_NEWSPAPER_READING=3,SHIELDTYPE_NEWSPAPER=1;
constexpr int PHASE_LADDER_CARRYING=4,PHASE_LADDER_PLACING=5;
constexpr int PHASE_POLEVAULTER_PRE_VAULT=6,PHASE_POLEVAULTER_POST_VAULT=7;
constexpr int PHASE_BALLOON_FLYING=8;
constexpr int STATE_NOTREADY=0,STATE_SQUASH_DONE_FALLING=50;
enum ReanimationType {REANIM_ZOMBIE,REANIM_FLAG,REANIM_REPEATER};
enum DrawVariation {VARIATION_NORMAL,VARIATION_IMITATER};
enum ProjectileType {PROJECTILE_PEA,PROJECTILE_SNOWPEA,PROJECTILE_FIREBALL,PROJECTILE_ZOMBIE_PEA,PROJECTILE_SPIKE,PROJECTILE_BUTTER,PROJECTILE_KERNEL,PROJECTILE_CABBAGE,PROJECTILE_STAR,PROJECTILE_PUFF};
enum ProjectileMotion {MOTION_STRAIGHT,MOTION_STAR,MOTION_HOMING,MOTION_THREEPEATER,MOTION_BACKWARDS};
constexpr int REANIM_PLAY_ONCE_AND_HOLD=0,REANIM_LOOP=1;
constexpr int FOLEY_THROW=0;
constexpr int FOLEY_SPAWN_SUN=1,COIN_SUN=0,COIN_MOTION_FROM_PLANT=0;
enum PlantWeapon {WEAPON_PRIMARY,WEAPON_SECONDARY};
enum PlantSubClass {SUBCLASS_NORMAL,SUBCLASS_SHOOTER};
constexpr int RENDER_GROUP_HIDDEN=-1,RENDER_GROUP_NORMAL=0,DS_ALIGN_CENTER=0;
namespace Sexy {
inline int forcedRoll=-1;inline uint32_t randomSeed=123;
inline int Rand(int n){if(forcedRoll>=0)return forcedRoll%n;randomSeed=randomSeed*1664525u+1013904223u;return (randomSeed>>1)%n;}
struct Color{int mRed,mGreen,mBlue,mAlpha;Color(int r=0,int g=0,int b=0,int a=255):mRed(r),mGreen(g),mBlue(b),mAlpha(a){}};
struct SexyTransform2D{float m00=1,m01=0,m02=0,m10=0,m11=1,m12=0;void LoadIdentity(){*this={};}};
struct Rect{int mX,mY,mWidth,mHeight;Rect(int x=0,int y=0,int w=0,int h=0):mX(x),mY(y),mWidth(w),mHeight(h){}};
struct Image{int mWidth=80,mHeight=80;std::string path;virtual~Image()=default;};
struct MemoryImage:Image{std::vector<uint32_t> bits=std::vector<uint32_t>(6400,0xffffffff);uint32_t* GetBits(){return bits.data();}void Create(int w,int h){mWidth=w;mHeight=h;bits.resize(w*h);}void BitsChanged(){}};
struct GLImage:MemoryImage{};
struct RectDraw{Rect bounds;Color color;};
inline std::vector<RectDraw> drawnRects;
struct Graphics{
 float mTransX=0,mTransY=0,mScaleX=1,mScaleY=1;Rect mClipRect;int mDrawMode=0;
 Color mColor;
 Graphics(MemoryImage*){};Graphics(const Graphics&)=default;
 void SetLinearBlend(bool){};void SetColor(Color c){mColor=c;};void DrawLine(int,int,int,int){};void FillRect(int x,int y,int w,int h){drawnRects.push_back({{x,y,w,h},mColor});};void SetClipRect(int,int,int,int){};
 void ClipRect(int,int,int,int){};
 void DrawImage(Image*,Rect,Rect){};
};
inline Image* IMAGE_SEEDS=nullptr;inline int FONT_BRIANNETOD12=0;
inline Image* IMAGE_PROJECTILEPEA=nullptr;
}
struct Track{const char* mName="";};
struct TrackGroup{int count=0;Track* tracks=nullptr;};
struct Definition{TrackGroup mTracks;};
struct ReanimatorTrackInstance{Sexy::Image* mImageOverride=nullptr;int mRenderGroup=0;};
using TrackInstance=ReanimatorTrackInstance;
inline std::vector<Sexy::Image*> drawnOverrides;
struct ReanimatorTransform{float mFrame=0,mAlpha=1,mTransX=0,mTransY=0,mScaleX=1,mScaleY=1,mSkewX=0,mSkewY=0;Sexy::Image* mImage=nullptr;};
struct Reanimation{
 Definition def;Definition* mDefinition=&def;TrackInstance* mTrackInstances=nullptr;float mAnimTime=0;
 std::string track;Sexy::SexyTransform2D matrix;ReanimatorTransform pose;
 void ReanimationInitializeType(int,int,ReanimationType){};bool TrackExists(const char* name){return track==name;}
 void PlayReanim(const char*,int,int,float){}
 void SetFramesForLayer(const char*){};void Draw(Sexy::Graphics*){for(int i=0;i<def.mTracks.count;++i)drawnOverrides.push_back(mTrackInstances[i].mImageOverride);};void SetImageOverride(const char* name,Sexy::Image* image){for(int i=0;i<def.mTracks.count;++i)if(std::string(def.mTracks.tracks[i].mName)==name)mTrackInstances[i].mImageOverride=image;};Reanimation* FindSubReanim(ReanimationType){return nullptr;}
 void AssignRenderGroupToPrefix(const char* prefix,int group){for(int i=0;i<def.mTracks.count;++i)if(std::string(def.mTracks.tracks[i].mName).starts_with(prefix))mTrackInstances[i].mRenderGroup=group;}
 int mFrameBasePose=0;Sexy::SexyTransform2D mOverlayMatrix;int FindTrackIndex(const char*){return 0;}void GetAttachmentOverlayMatrix(int,Sexy::SexyTransform2D&){};
 void GetTrackMatrix(int,Sexy::SexyTransform2D& out){out=matrix;}void GetCurrentTransform(int,ReanimatorTransform* out){*out=pose;}
};
struct ReanimatorCache{
 std::unique_ptr<Sexy::MemoryImage> MakeBlankMemoryImage(int w,int h){auto im=std::make_unique<Sexy::MemoryImage>();im->Create(w,h);return im;}
 std::unique_ptr<Sexy::MemoryImage> MakeCachedPlantFrame(SeedType,DrawVariation){return MakeBlankMemoryImage(120,120);}
 void UpdateReanimationForVariation(Reanimation*,DrawVariation){}
};
struct LawnApp{std::vector<std::pair<int,float>> memeCues;void PlayMemeCue(int cue,float pitch=0){memeCues.push_back({cue,pitch});}int rageReleaseRequests=0;void PlayRageRelease(){++rageReleaseRequests;}bool adventure=true;bool IsAdventureMode(){return adventure;}ReanimatorCache cache;ReanimatorCache* mReanimatorCache=&cache;std::map<int,Reanimation*> reanims;Reanimation* ReanimationTryToGet(int id){return reanims.contains(id)?reanims.at(id):nullptr;}Sexy::GLImage* GetImage(std::string file){auto* im=new Sexy::GLImage;im->path=file;return im;}void PlayFoley(int){}};
extern LawnApp* gLawnApp;
class Board;
class Zombie{
public:
 static constexpr int ZOMBIE_WAVE_DEBUG=-1;
 Board* mBoard=nullptr;ZombieType mZombieType=ZOMBIE_NORMAL;ZombieID id=ZOMBIEID_NULL;
 float mPosX=0,mPosY=0,mScaleZombie=1;int mTargetCol=0,mX=0,mY=0,mRow=0,mBodyReanimID=0,mBodyHealth=1000,mBodyMaxHealth=1000,mHelmHealth=0,mHelmMaxHealth=0;
 bool mDead=false,mMindControlled=false,mHasHead=true,mHasArm=true,mIsEating=false,mBlowingAway=false;int chill=0,mIceTrapCounter=0,mButteredCounter=0,mRenderOrder=0;
 bool mHasObject=false;Sexy::Rect mZombieAttackRect{50,0,20,115};int mTargetRow=-1,mSummonCounter=0;
 int mZombiePhase=0,mZombieHeight=0;bool mInPool=false;
 int mPhaseCounter=0,mShieldHealth=0,mShieldMaxHealth=0,mShieldType=0,mZombieAge=0,mFromWave=0;float mAltitude=0;bool flying=false;
 bool IsFlying(){return flying;}
 void StopEating(){mIsEating=false;}void AttachShield(){}void PickRandomSpeed(){}
 void SetRow(int row){mRow=row;}
 float GetPosYBasedOnRow(int row){return row*100.0f;}
 bool IsOnBoard(){return true;}
 void StartWalkAnim(int){}
 int headHides=0;
 void SetupReanimForLostHead(){++headHides;if(auto* anim=gLawnApp->ReanimationTryToGet(mBodyReanimID))for(const char* prefix:{"anim_head","anim_hair","anim_tongue"})anim->AssignRenderGroupToPrefix(prefix,RENDER_GROUP_HIDDEN);}
 void PlayZombieReanim(const char*,int,int,float){}
 bool IsImmobilizied(){return mIceTrapCounter>0||mButteredCounter>0;}
 bool movementBlocked=false;
 bool ZombieNotWalking(){return mIsEating||IsImmobilizied()||movementBlocked;}
 bool IsMovingAtChilledSpeed(){return chill>0;}
 void UpdateAnimSpeed(){}
 void ReanimShowPrefix(const char*,int){}
 int mSpecialHeadReanimID=0;
 unsigned mTargetPlantID=0;
 bool IsDeadOrDying(){return mDead||mBodyHealth<=0;};bool EffectedByDamage(unsigned){return !IsDeadOrDying();}
 void TakeDamage(int n,unsigned){int shield=std::min(n,mShieldHealth);mShieldHealth-=shield;n-=shield;int armor=std::min(n,mHelmHealth);mHelmHealth-=armor;mBodyHealth-=n-armor;}
 void UpdateReanim(){};void RemoveColdEffects(){chill=0;}
 void ApplyChill(bool){chill=600;}
 static void PreloadZombieResources(ZombieType){};static void SetupReanimLayers(Reanimation*,ZombieType){}
};
class Plant{
public:
 Board* mBoard=nullptr;SeedType mSeedType=SEED_PEASHOOTER;
 SeedType mImitaterType=SEED_NONE;int mRecentlyEatenCountdown=0;
 int mX=0,mY=0,mRow=0,mPlantCol=0,mPlantHealth=300,mPlantMaxHealth=300,mLaunchRate=150,mLaunchCounter=100,mBlinkCountdown=0,mShootingCounter=0;
 int mBodyReanimID=0,mHeadReanimID=0,mHeadReanimID2=0,mHeadReanimID3=0,mBlinkReanimID=0;
 int mRenderOrder=0,mEatenFlashCountdown=0,mState=STATE_READY,mStateCountdown=0;
 int mTargetX=0;
 int CalcRenderOrder(){return mRow*100;}void PlayBodyReanim(const char*,int,int,float){}
 bool mDead=false,mIsAsleep=false,mSquished=false,airborne=false;
 float drawHeightOffset=0;
 bool NotOnGround(){return airborne;}
 void SetSleeping(bool value){mIsAsleep=value;}void Die(){mDead=true;}
 int GetDamageRangeFlags(PlantWeapon){return 0;};Zombie* FindTargetZombie(int row,PlantWeapon);
 void Fire(Zombie*,int row,PlantWeapon);
 bool FindTargetAndFire(int row,PlantWeapon);
};
class Projectile;
namespace SandboxPlants {void ForgetShot(Projectile*);void OnFired(Plant*,Projectile*,Zombie*);}
namespace SandboxZombies {void ForgetShot(Projectile*);}
class Projectile{
public:
 Board* mBoard=nullptr;bool mDead=false;ProjectileMotion mMotionType=MOTION_STRAIGHT;ZombieID mTargetZombieID=ZOMBIEID_NULL;
 float mPosX=0,mPosY=0,mPosZ=0,mVelX=3.3,mVelY=0,mShadowY=0;
 int mX=0,mY=0,mRow=0,mRenderOrder=0,mDamageRangeFlags=0;
 int mProjectileAge=0;
 void Die(){mDead=true;SandboxPlants::ForgetShot(this);SandboxZombies::ForgetShot(this);}
 ProjectileType mProjectileType=PROJECTILE_PEA;
 void ConvertToFireball(int){mProjectileType=PROJECTILE_FIREBALL;}
 int GetDamageFlags(Zombie*){return 0;}
};
template<class T>struct Array{
 std::vector<T*> values;int mSize=0,mMaxSize=256;
 auto begin(){return values.begin();}auto end(){return values.end();}
 void add(T* t){values.push_back(t);++mSize;}
 unsigned DataArrayGetID(T* t){for(unsigned i=0;i<values.size();++i)if(values[i]==t)return i+1;return 0;}
 T* DataArrayTryToGet(unsigned id){return id>0&&id<=values.size()?values[id-1]:nullptr;}
};
class Board{
 unsigned nextID=1;
public:
 struct Packet {int mPacketType=-1,mImitaterType=-1,mRefreshCounter=0,mRefreshTime=750;bool mRefreshing=false;};
 struct Bank {int mNumPackets=0;Packet mSeedPackets[10];} bank;
 Bank* mSeedBank=&bank;
 bool mPaused=false,pool=false,night=false;int mMainCounter=0;
 bool StageIsNight(){return night;}
 Array<Plant> mPlants;Array<Zombie> mZombies;Array<Projectile> mProjectiles;
 struct {int mSize=0,mMaxSize=256;} mCoins;
 void AddCoin(int,int,int,int){++mCoins.mSize;}
 std::vector<std::unique_ptr<Plant>> ownedPlants;std::vector<std::unique_ptr<Zombie>> ownedZombies;std::vector<std::unique_ptr<Projectile>> ownedShots;
 bool StageHasPool(){return pool;}
 int GridToPixelX(int col,int){return col*80;}int GridToPixelY(int,int row){return row*100;}
 int PixelToGridXKeepOnBoard(int x,int){return std::clamp(x/80,0,8);}
 bool RowCanHaveZombies(int row){return row>=0&&row<(pool?6:5);}
 ZombieID ZombieGetID(Zombie* z){return z?z->id:ZOMBIEID_NULL;}
 Zombie* ZombieTryToGet(ZombieID id){for(auto* z:mZombies)if(z->id==id)return z;return nullptr;}
 Zombie* AddZombieInRow(ZombieType type,int row,int){
  auto p=std::make_unique<Zombie>();auto* z=p.get();z->mBoard=this;z->id=ZombieID(nextID++);z->mZombieType=type;z->mRow=row;z->mPosY=row*100;ownedZombies.push_back(std::move(p));mZombies.add(z);return z;
 }
 Projectile* AddProjectile(float x,float y,int order,int row,ProjectileType type){
  auto p=std::make_unique<Projectile>();auto* s=p.get();s->mBoard=this;s->mProjectileType=type;s->mPosX=x;s->mPosY=y;s->mRenderOrder=order;s->mRow=row;ownedShots.push_back(std::move(p));mProjectiles.add(s);return s;
 }
 Plant* plant(int col,int row){auto p=std::make_unique<Plant>();auto* a=p.get();a->mBoard=this;a->mPlantCol=col;a->mRow=row;a->mX=col*80;a->mY=row*100;ownedPlants.push_back(std::move(p));mPlants.add(a);return a;}
};
inline Zombie* Plant::FindTargetZombie(int row,PlantWeapon weapon){for(auto* z:mBoard->mZombies)if(!z->IsDeadOrDying()&&z->mRow==row&&!z->mMindControlled&&(int(mSeedType)==28&&weapon==WEAPON_SECONDARY?z->mPosX<mX:z->mPosX>=mX)&&(int(mSeedType)!=10||z->mPosX<mX+400))return z;return nullptr;}
inline void Plant::Fire(Zombie* target,int row,PlantWeapon weapon){
 if(int(mSeedType)==10){for(auto* z:mBoard->mZombies)if(!z->IsDeadOrDying()&&!z->mMindControlled&&z->mRow==row&&z->mPosX>=mX&&z->mPosX<mX+400)z->TakeDamage(20,2);return;}
 const int base=int(mSeedType);const auto type=base==5?PROJECTILE_SNOWPEA:base==26?PROJECTILE_SPIKE:base==34?(weapon==WEAPON_SECONDARY?PROJECTILE_BUTTER:PROJECTILE_KERNEL):base==32?PROJECTILE_CABBAGE:base==13?PROJECTILE_PUFF:PROJECTILE_PEA;
 auto* s=mBoard->AddProjectile(mX+60,mY+25,mRenderOrder,row,type);if(base==28&&weapon==WEAPON_SECONDARY)s->mMotionType=MOTION_BACKWARDS;SandboxPlants::OnFired(this,s,target);
}
inline bool Plant::FindTargetAndFire(int row,PlantWeapon weapon){auto* target=FindTargetZombie(row,weapon);if(!target)return false;Fire(target,row,weapon);return true;}
inline float PlantDrawHeightOffset(Board*,Plant* p,SeedType,int,int){return p?p->drawHeightOffset:0;}
struct PlantDefinition{ReanimationType mReanimationType=REANIM_ZOMBIE;PlantSubClass mSubClass=SUBCLASS_SHOOTER;int mLaunchRate=150;};
inline PlantDefinition GetPlantDefinition(SeedType type){return {REANIM_ZOMBIE,int(type)==1||int(type)==3?SUBCLASS_NORMAL:SUBCLASS_SHOOTER};}
inline void DrawSeedPacket(Sexy::Graphics*,int,int,SeedType,SeedType,int,int,bool,bool){}
inline void PvzpDrawImageCelScaledF(Sexy::Graphics*,Sexy::Image*,int,int,int,int,int,int){}
inline void PvzpDrawImageScaledF(Sexy::Graphics*,Sexy::Image*,float,float,float,float){}
inline void PvzpDrawString(Sexy::Graphics*,const char*,int,int,int,Sexy::Color,int){}
inline void PvzpDrawString(Sexy::Graphics*,const std::string&,int,int,int,Sexy::Color,int){}
struct Blit{std::string path;Sexy::SexyTransform2D matrix;int alpha;};
inline std::vector<Blit> testBlits;
inline void PvzpBltMatrix(Sexy::Graphics*,Sexy::Image* im,const Sexy::SexyTransform2D& mat,Sexy::Rect,Sexy::Color color,int,Sexy::Rect){testBlits.push_back({im->path,mat,color.mAlpha});}
