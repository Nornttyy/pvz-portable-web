// Original local sandbox extension. SPDX-License-Identifier: LGPL-3.0-or-later
#include "Sandbox.h"
#include "MemeAdventure.h"
#include "SandboxRules.h"
#include "SandboxFusion.h"
#include "SandboxUIRules.h"
#include "SandboxScenes.h"
#include "SandboxFactions.h"
#include "LawnApp.h"
#include "Lawn/Board.h"
#include "Lawn/Plant.h"
#include "Lawn/Zombie.h"
#include "Lawn/Projectile.h"
#include "Lawn/Coin.h"
#include "Lawn/GridItem.h"
#include "Lawn/LawnMower.h"
#include "Lawn/Cutscene.h"
#include "Lawn/SeedPacket.h"
#include "Lawn/System/PlayerInfo.h"
#include "Lawn/Widget/GameButton.h"
#include "Lawn/System/Music.h"
#include "graphics/GLInterface.h"
#include "widget/WidgetManager.h"
#include <SDL.h>
#include <algorithm>
#include <memory>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

bool gSandboxEnabled = false;
static bool paused = true, stepOnce = false, awake = true;
static bool stackPlants = false, continuousZombies = false;
static bool fusionEnabled = false;
static bool charmPlants=false,charmZombies=false;
static int mapType = 0, escaped = 0;
static int sessionRevision = 0;
static std::unique_ptr<PlayerInfo> sandboxProfile;
static PlayerInfo* adventureProfile = nullptr;
static GameMode previousMode = GAMEMODE_ADVENTURE;
static bool previousEasyPlanting = false;
static double previousSpeed = 1;

static void CanvasSize(int width) {
    auto* app=gLawnApp;
    app->mWidth=width;app->mHeight=600;
    app->mScreenBounds=Sexy::Rect(0,0,width,600);
    SDL_SetWindowSize(static_cast<SDL_Window*>(app->mWindow),width,600);
    app->mGLInterface->ResizeLogicalCanvas(width,600);
    app->mWidgetManager->Resize(app->mScreenBounds,app->mGLInterface->mPresentationRect);
#ifdef __EMSCRIPTEN__
    EM_ASM({ window.dispatchEvent(new Event('resize')); });
#endif
}

bool SandboxOwnsProfile(const PlayerInfo* profile) {
    return profile && profile == sandboxProfile.get();
}

bool SandboxEnter() {
    auto* app = gLawnApp;
    if (!app || gSandboxEnabled || app->mGameScene != SCENE_MENU || app->mBoard || app->GetDialogCount() > 0) return false;
    // Keep the real profile owned by ProfileMgr intact, including unfinished saves.
    adventureProfile = app->mPlayerInfo;
    previousMode = app->mGameMode;
    previousEasyPlanting = app->mEasyPlantingCheat;
    previousSpeed = app->mUpdateMultiplier;
    sandboxProfile = std::make_unique<PlayerInfo>();
    // Practice with a disposable copy, never debit the actual adventure wallet.
    if (adventureProfile) sandboxProfile->mCoins = adventureProfile->mCoins;
    gSandboxEnabled = true;
    awake = true;
    stackPlants = false;
    continuousZombies = false;
    fusionEnabled = false;
    charmPlants=charmZombies=false;
    CanvasSize(SandboxUIRules::CanvasWidth);
    SandboxStart(0);
    return true;
}

bool SandboxExit() {
    auto* app = gLawnApp;
    if (!app || !gSandboxEnabled) return false;
    // Dispose the sandbox while save/delete guards are still active.
    app->mBoardResult = BOARDRESULT_NONE;
    SandboxUIDetach();
    app->KillBoard();
    SandboxPlants::Reset();
    SandboxZombies::Reset();
    SandboxScenes::Reset();
    SandboxFactions::Reset();
    app->mPlayerInfo = adventureProfile;
    adventureProfile = nullptr;
    app->mGameMode = previousMode;
    app->mEasyPlantingCheat = previousEasyPlanting;
    app->mUpdateMultiplier = previousSpeed;
    gSandboxEnabled = false;
    CanvasSize(800);
    app->ShowGameSelector();
    return true;
}

static Board* ActiveBoard() {
    return gSandboxEnabled && gLawnApp && gLawnApp->mGameScene == SCENE_PLAYING ? gLawnApp->mBoard : nullptr;
}

void SandboxStart(int map) {
    if (!gSandboxEnabled || !gLawnApp || !SandboxRules::ValidMap(map)) return;
    auto* app = gLawnApp;
    app->mBoardResult = BOARDRESULT_NONE;
    app->KillGameSelector();
    app->KillSeedChooserScreen();
    SandboxUIDetach();
    app->KillBoard();
    SandboxPlants::Reset();
    SandboxZombies::Reset();
    SandboxScenes::Reset();
    SandboxFactions::Reset();
    if (!sandboxProfile) sandboxProfile = std::make_unique<PlayerInfo>();
    auto* profile = sandboxProfile.get();
    profile->mName = "Sandbox";
    profile->mId = 1;
    profile->mFinishedAdventure = 1;
    profile->mLevel = SandboxSceneRules::Scenes[map].level;
    app->mPlayerInfo = profile;
    app->mGameMode = GAMEMODE_ADVENTURE;
    app->mEasyPlantingCheat = true;
    app->MakeNewBoard();
    auto* board = app->mBoard;
    board->InitLevel();
    if(map!=0&&map!=1)SandboxScenes::Switch(board,map,awake);
    board->mCutScene->PreloadResources();
    board->mCutScene->mPlacedLawnItems = true;
    board->mCutScene->mPlacedZombies = true;
    board->mCutScene->mCutsceneTime = 100000;
    board->mCutScene->mSeedChoosing = false;
    board->mSeedBank->mVisible = false;
    board->mSeedBank->mNumPackets = 0;
    board->mMenuButton->mBtnNoDraw = true;
    board->mMenuButton->mDisabled = true;
    if (board->mStoreButton) { board->mStoreButton->mBtnNoDraw = true; board->mStoreButton->mDisabled = true; }
    board->mShowShovel = false;
    board->mSodPosition = 0;
    board->mSunMoney = 9999;
    board->mTutorialState = TUTORIAL_OFF;
    board->mEnableGraveStones = false;
    // Native world remains 800 x 600. Only its widget origin moves; physics,
    // projectile transforms, mouse coordinates and saves stay in native units.
    board->Resize(SandboxUIRules::WorldOffset,0,800,600);
    board->ClearAdvice(ADVICE_NONE);
    app->mGameScene = SCENE_PLAYING;
    app->mBoardResult = BOARDRESULT_NONE;
    board->StartLevel();
    mapType = map;
    escaped = 0;
    paused = true;
    stepOnce = false;
    app->mUpdateMultiplier = 1;
    board->mPaused = true;
    ++sessionRevision;
    SandboxUIReset();
    board->MarkAllDirty();
}

void SandboxTick(Board* board) {
    if (!gSandboxEnabled) { MemeAdventure::Tick(board); return; }
    SandboxUITick(board);
    board->mSunMoney = 9999;
    board->mPaused = paused && !stepOnce;
    SandboxPlants::Tick(board);
    SandboxZombies::Tick(board);
    stepOnce = false;
}
void SandboxEscaped() { ++escaped; }
static int PlantCount(Board* board) {
    int count = 0;
    for (auto* plant : board->mPlants) if (!plant->mDead) ++count;
    return count;
}
static int ZombieCount(Board* board) {
    int count = 0;
    for (auto* zombie : board->mZombies) if (!zombie->IsDeadOrDying()) ++count;
    return count;
}
static void ClearEnemies(Board* board) {
    for (auto* zombie : board->mZombies) if (!zombie->mDead) zombie->DieNoLoot();
    for (auto* shot : board->mProjectiles) if (!shot->mDead) shot->Die();
    board->ProcessDeleteQueue();
    SandboxZombies::Reset();
}
// Infuse in place. Prefer an unpowered main plant, then shell, then support.
static Plant* FindFusionTarget(Board* board,int type,int col,int row,int& result) {
    result=0;
    if(!fusionEnabled||stackPlants||!SandboxRules::ValidCard(type)||!SandboxRules::ValidCell(col,row,SandboxSceneRules::Pool(mapType)))return nullptr;
    Plant* target=nullptr;
    for(auto* p:board->mPlants){
        if(p->mDead||p->mRow!=row||p->NotOnGround())continue;
        if(p->mPlantCol!=col&&!(p->mSeedType==SEED_COBCANNON&&p->mPlantCol==col-1))continue;
        if(p->mPlantHealth<=0||!SandboxFusion::Result(SandboxPlants::Type(p),type))continue;
        const auto rank=[](Plant* plant){const int base=plant->mSeedType;return base==35?4:base==30?2:base==16||base==33?1:3;};
        if(!target||rank(p)>rank(target))target=p;
    }
    if(!target||target->mPlantHealth<=0)return nullptr;
    const int candidate=SandboxFusion::Result(SandboxPlants::Type(target),type);
    if(!candidate)return nullptr;
    result=candidate;
    return target;
}
static int PlacePlant(Board* board, int type, int col, int row, bool preserved=false) {
    if (!SandboxRules::ValidCard(type) || !SandboxRules::ValidCell(col, row, SandboxSceneRules::Pool(mapType))) return -2;
    int fusedType=0;
    if(auto* target=FindFusionTarget(board,type,col,row,fusedType)){
        if(SandboxMemeRules::IsPower(type)){
            // Imbue the existing instance. Health, damage layers, supports and
            // its native animation attachments remain intact; no heal/replant.
            SandboxPlants::Assign(target,fusedType);board->MarkAllDirty();return fusedType;
        }
        if (board->mPlants.mSize >= board->mPlants.mMaxSize - 8) return -3;
        const auto resultSeed=static_cast<SeedType>(SandboxPlants::Base(fusedType));
        Plant::PreloadPlantResources(resultSeed);
        // Allocate before consuming anything; a failed placement must be harmless.
        auto* fused=board->AddPlant(col,row,resultSeed,SEED_NONE);
        if(!fused)return -3;
        SandboxPlants::Assign(fused,fusedType);
        fused->mPlantHealth=SandboxFusion::InheritedHealth(target->mPlantHealth,target->mPlantMaxHealth,fused->mPlantMaxHealth);
        target->Die();
        if(awake&&fused->mIsAsleep)fused->SetSleeping(false);
        board->MarkAllDirty();
        return fusedType;
    }
    if (SandboxMemeRules::IsPower(type)) return -6; // Powers never become standalone plants.
    if (board->mPlants.mSize >= board->mPlants.mMaxSize - 8) return -3;
    const auto seed = static_cast<SeedType>(SandboxPlants::Base(type));
    if (seed == SEED_COBCANNON && col >= 8) return -4;
    if (int(seed) == 8 && MemeCharacters::PuffCount(board,col,row) >= TinyPuffRules::Limit) return -4;
    if(preserved){Plant::PreloadPlantResources(seed);auto* plant=board->AddPlant(col,row,seed,SEED_NONE);if(!plant)return -3;SandboxPlants::Assign(plant,type);SandboxFactions::Set(plant,charmPlants);if(awake&&plant->mIsAsleep)plant->SetSleeping(false);board->MarkAllDirty();return 1;}
    if (PlantCount(board) >= SandboxRules::MaxPlants) return -3;
    if (seed == SEED_CATTAIL && !board->IsPoolSquare(col, row)) return -4;
    if (stackPlants && seed != SEED_GRAVEBUSTER && seed != SEED_INSTANT_COFFEE) {
        SandboxRules::StackSite site;
        site.water = board->IsPoolSquare(col, row);
        site.blocked = board->GetGraveStoneAt(col,row) || board->GetCraterAt(col,row) || board->GetScaryPotAt(col,row) || board->IsIceAt(col,row);
        for (auto* existing : board->mPlants) {
            if (existing->mDead || existing->mRow != row || existing->NotOnGround()) continue;
            if (existing->mPlantCol == col) {
                site.lily |= existing->mSeedType == SEED_LILYPAD;
                site.pot |= existing->mSeedType == SEED_FLOWERPOT;
                site.cattail |= existing->mSeedType == SEED_CATTAIL;
            }
            if (existing->mPlantCol == col+1) site.rightLily |= existing->mSeedType == SEED_LILYPAD;
        }
        if (!SandboxRules::StackTerrainAllows(seed,col,site)) return -4;
    } else if (board->CanPlantAt(col, row, seed) != PLANTING_OK) return -4;
    Plant::PreloadPlantResources(seed);
    PlantsOnLawn existing{};
    board->GetPlantsOnLawn(col, row, &existing);
    if (!stackPlants && Plant::IsUpgrade(seed)) {
        if (seed == SEED_CATTAIL && existing.mUnderPlant) existing.mUnderPlant->Die();
        else if (existing.mNormalPlant && existing.mNormalPlant->IsUpgradableTo(seed)) existing.mNormalPlant->Die();
        if (seed == SEED_COBCANNON) {
            PlantsOnLawn second{};
            board->GetPlantsOnLawn(col + 1, row, &second);
            if (second.mNormalPlant) second.mNormalPlant->Die();
        }
    }
    auto* plant = board->AddPlant(col, row, seed, SEED_NONE);
    SandboxPlants::Assign(plant,type);
    SandboxFactions::Set(plant,charmPlants);
    if (awake && plant->mIsAsleep) plant->SetSleeping(false);
    board->MarkAllDirty();
    return 1;
}
static int Spawn(Board* board, int type, int col, int row) {
    if (!SandboxRules::ValidZombie(type) || !SandboxRules::ValidCell(col, row, SandboxSceneRules::Pool(mapType))) return -2;
    if (ZombieCount(board) >= SandboxRules::MaxZombies || board->mZombies.mSize >= board->mZombies.mMaxSize - 8) return -3;
    const auto requestedType = static_cast<ZombieType>(SandboxZombies::Base(type));
    // Native swimming is the ordinary zombie plus its row-dependent duck rig.
    // The catalogue-only DUCKY_TUBE type is not accepted by native pool motion.
    auto zombieType = requestedType == ZOMBIE_DUCKY_TUBE ? ZOMBIE_NORMAL : requestedType;
    Zombie::PreloadZombieResources(zombieType);
    auto* zombie = board->AddZombieInRow(zombieType, row, Zombie::ZOMBIE_WAVE_DEBUG);
    if (!zombie) return -3;
    if(zombieType!=ZOMBIE_BOSS){const float oldX=zombie->mPosX;zombie->mPosX = static_cast<float>(board->GridToPixelX(col, row) + 10);zombie->mX = static_cast<int>(zombie->mPosX);SandboxScenes::MoveFollowers(zombie,zombie->mPosX-oldX);}
    SandboxZombies::Assign(zombie,type);
    SandboxFactions::Set(zombie,charmZombies);
    SandboxScenes::UpdateSwimmer(zombie);
    zombie->UpdateReanim();
    board->MarkAllDirty();
    return 1;
}

extern "C" EMSCRIPTEN_KEEPALIVE int pvz_sandbox_command(int command, int type, int col, int row) {
    auto* board = ActiveBoard();
    if (!board) return -1;
    switch (command) {
    case 0: return 1 | (paused ? 2 : 0) | (SandboxSceneRules::Pool(mapType) ? 4 : 0) | (awake ? 8 : 0) | (stackPlants ? 16 : 0) | (continuousZombies ? 32 : 0) | (fusionEnabled ? 64 : 0) | (charmPlants?128:0) | (charmZombies?256:0);
    case 1: return PlacePlant(board, type, col, row);
    case 2: return Spawn(board, type, col, row);
    case 3: {
        if (!SandboxRules::ValidCell(col, row, SandboxSceneRules::Pool(mapType))) return -2;
        Plant* top = nullptr;
        // The five-mushroom cluster is natural stacking: shovel one member,
        // not all five or their lily pad, even when global stacking is off.
        if (!stackPlants) {
            for (auto* plant : board->mPlants) if (!plant->mDead && plant->mRow==row && plant->mPlantCol==col && MemeCharacters::Type(plant)==MemeCharacters::TinyPuff) top=plant;
            if (top) { top->Die(); board->ProcessDeleteQueue(); board->MarkAllDirty(); return 1; }
        }
        for (auto* plant : board->mPlants) if (!plant->mDead && plant->mRow == row && (plant->mPlantCol == col || (plant->mSeedType == SEED_COBCANNON && plant->mPlantCol + 1 == col))) {
            if (stackPlants) top = plant;
            else plant->Die();
        }
        if (top) top->Die();
        for (auto* item : board->mGridItems) if (!item->mDead && item->mGridX == col && item->mGridY == row) item->GridItemDie();
        board->ProcessDeleteQueue(); board->MarkAllDirty(); return 1;
    }
    case 4: paused = type != 0; board->mPaused = paused; return 1;
    case 5:
        if (!SandboxRules::ValidSpeed(type)) return -2;
        gLawnApp->mUpdateMultiplier = type; return 1;
    case 6: ClearEnemies(board); return 1;
    case 7:
        SandboxStart(mapType); return 1; // Also resets ice, craters and ongoing instant effects.
    case 8:
        if(!SandboxRules::ValidMap(type))return -2;
        if(type==mapType)return 1;
        if(!SandboxScenes::Switch(board,type,awake))return -3;
        mapType=type;++sessionRevision;return 1;
    case 9: return PlantCount(board);
    case 10: return ZombieCount(board);
    case 11: {
        if (!SandboxRules::ValidZombie(type)) return -2;
        int added = 0;
        for (int y = 0; y < SandboxSceneRules::Rows(mapType); ++y) if (Spawn(board, type, 8, y) > 0) ++added;
        return added ? added : -5;
    }
    case 12:
        awake = type != 0;
        if (awake) for (auto* plant : board->mPlants) if (!plant->mDead && plant->mIsAsleep) plant->SetSleeping(false);
        return 1;
    case 13: paused = true; stepOnce = true; return 1;
    case 14: return escaped;
    case 15: return SandboxExit() ? 1 : -1;
    case 16: SandboxUIFeedback(type); return 1;
    case 17: return SandboxUIHasUnsaved() ? 1 : 0;
    case 18: return sessionRevision;
    case 19: stackPlants = type != 0; if(stackPlants)fusionEnabled=false; return 1;
    case 20: continuousZombies = type != 0; return 1;
    case 21: fusionEnabled = false; return 1; // Retired layout flag, accepted for compatibility only.
    case 22: { int result=0; FindFusionTarget(board,type,col,row,result); return result; }
    case 23:
        for(auto* p:board->mPlants)if(!p->mDead&&p->mPlantCol==col&&p->mRow==row&&MemeCharacters::Activate(p,type))return 1;
        return 0;
    case 24: return mapType;
    case 25: if(!SandboxRules::ValidMap(type))return -2;SandboxStart(type);return 1;
    case 27: return PlacePlant(board,type,col,row,true);
    case 28: charmPlants=type!=0;return 1;
    case 29: charmZombies=type!=0;return 1;
    default: return -2;
    }
}

// Read-only diagnostics: verify collection/credit without touching a player's balance.
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_coin_data(int index, int field) {
    auto* board = gLawnApp ? gLawnApp->mBoard : nullptr;
    if (!board || !gLawnApp->mPlayerInfo) return -1;
    if (index == -1) return field == 0 ? gLawnApp->mPlayerInfo->mCoins : field == 1 ? board->mCoinsCollected : field == 2 ? board->mLevelCoinsCollected : -1;
    if (index < 0) return -1;
    for (auto* coin : board->mCoins) if (!coin->mDead && index-- == 0)
        return field == 0 ? int(coin->mType) : field == 1 ? coin->mCoinAge : field == 2 ? int(coin->mIsBeingCollected) : field == 3 ? int(coin->mPosX) : field == 4 ? int(coin->mPosY) : field == 5 ? coin->mFadeCount : field == 6 ? Coin::GetCoinValue(coin->mType) : field == 7 ? int(board->mCoins.DataArrayGetID(coin)) : -1;
    return -1;
}

extern "C" EMSCRIPTEN_KEEPALIVE int pvz_sandbox_zombie_data(int index,int field) {
    // Read-only diagnostics also cover adventure save/resume. Commands remain
    // sandbox-only; this cannot spawn enemies or alter campaign progress.
    auto* board=gLawnApp?gLawnApp->mBoard:nullptr;if(!board||index<0||field<0||field>30)return -1;
    if(field==30){for(auto* z:board->mZombies)if(!z->mDead&&z->IsOnBoard())if(index--==0)return int(z->mMindControlled);return -1;}
    if(field>=25){for(auto* z:board->mZombies)if(!z->mDead&&z->IsOnBoard())if(index--==0)return field==25?int(SandboxZombies::IsForwardFlight(z)):field==26?z->mSummonCounter:field==27?z->mBossBungeeCounter:field==28?int(z->mPosX*1000):SandboxZombies::FlipDuration(z);return -1;}
    if(field>=21){for(auto* z:board->mZombies)if(!z->mDead&&z->IsOnBoard())if(index--==0)return field==21?int(z->mInPool):field==22?z->mBossHeadCounter-1:field==23?z->mBossStompCounter:z->mTargetRow;return -1;}
    if(field>=16){for(auto* z:board->mZombies)if(!z->mDead&&z->IsOnBoard())if(index--==0)return field==16?z->mChilledCounter:field==17?z->mIceTrapCounter:field==18?int(z->IsWalkingBackwards()):field==19?z->mTargetCol:int(z->mIsEating);return -1;}
    for(auto* z:board->mZombies)if(!z->mDead&&z->IsOnBoard()){if(index--==0)return field==0?SandboxZombies::Type(z):field==1?z->mRow:field==2?int(z->mPosX):field==3?int(z->mPosY):field==4?z->mBodyHealth:field==5?z->mHelmHealth:field==6?int(z->mZombiePhase):field==7?z->mShieldHealth:field==9?int(SandboxZombies::Speed(z)*100):field==10?z->mZombieAge:field==11?int(z->mAltitude):field==12?int(SandboxZombies::IsResting(z)):field==13?int(z->mHasHead&&!SandboxZombies::IsLouis(z)):field==14?int(z->mHasHead):field==15?int(z->mZombieType):z->mPhaseCounter;}
    return -1;
}

// Read-only sun diagnostics for both adventure and sandbox; no balance setters.
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_sun_data(int index,int field) {
    if(!gLawnApp||gLawnApp->mGameScene!=SCENE_PLAYING||!gLawnApp->mBoard||index<0||field<0||field>8)return -1;
    for(auto* sun:gLawnApp->mBoard->mCoins)if(!sun->mDead&&!sun->mIsBeingCollected&&sun->IsSun()){
        if(index--==0)return field==0?sun->GetSunValue():field==1?int(sun->mPosX+30):field==2?int(sun->mPosY+30):field==3?int(sun->mCoinMotion):field==4?sun->mCoinAge:field==5?int(sun->mType):field==6?sun->mDisappearCounter:field==7?int(sun->mScale*1000):int(sun->GetSunScale()*1000);
    }
    return -1;
}

// Read-only live projectile diagnostics, also available in adventure QA.
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_rage_audio_data(int field) {
    if(!gLawnApp||!gLawnApp->mSoundSystem)return -1;
    auto* audio=gLawnApp->mSoundSystem.get();
    if(field==0)return audio->IsFoleyPlaying(FOLEY_RAGE_RELEASE)?1:0;
    int count=0,volume=0;
    for(const auto& voice:audio->mFoleyTypeData[FOLEY_RAGE_RELEASE].mFoleyInstances)if(voice.mInstance&&voice.mRefCount&&!voice.mPaused&&voice.mInstance->IsPlaying()){
        ++count;volume=int(voice.mInstance->GetVolume()*1000);
    }
    return field==1?count:field==2?volume:-1;
}

extern "C" EMSCRIPTEN_KEEPALIVE int pvz_projectile_data(int index,int field) {
    auto* board=gLawnApp?gLawnApp->mBoard:nullptr;if(!board||index<0||field<0||field>8)return -1;
    for(auto* shot:board->mProjectiles)if(!shot->mDead&&index--==0){
        return field==0?MemeCharacters::ShotStyle(shot):field==1?int(shot->mPosX):field==2?int(shot->mPosY+shot->mPosZ):field==3?int(shot->mVelX*1000):field==4?int(shot->mVelY*1000):field==5?shot->mRow:field==6?shot->mProjectileAge:field==8?int(shot->mProjectileType):SandboxPlants::SaveShot(shot);
    }return -1;
}

// Read-only audio diagnostics: active cue, voice count and relative volume.
extern "C" EMSCRIPTEN_KEEPALIVE int pvz_meme_audio_data(int field) {
    if(!gLawnApp||!gLawnApp->mSoundSystem)return -1;
    int count=0,cue=-1,volume=0;
    for(int i=FOLEY_MEME_HICCUP;i<=FOLEY_MEME_SQUEAK;++i)
        for(const auto& voice:gLawnApp->mSoundSystem->mFoleyTypeData[i].mFoleyInstances)
            if(voice.mInstance&&voice.mRefCount&&!voice.mPaused&&voice.mInstance->IsPlaying()){
                ++count;cue=i-FOLEY_MEME_HICCUP;volume=int(voice.mInstance->GetVolume()*1000);
            }
    return field==0?cue:field==1?count:field==2?volume:-1;
}

extern "C" EMSCRIPTEN_KEEPALIVE int pvz_sandbox_plant_data(int index, int field) {
    auto* board = ActiveBoard();
    if (!board || index < 0 || field < 0 || field > 16) return -1;
    for (auto* plant : board->mPlants) {
        if (plant->mDead) continue;
        if (index-- == 0) {
            if(field==12)return int(board->mPlants.DataArrayGetID(plant));
            if(field==13)return plant->mX;
            if(field==14)return plant->mY;
            if(field==15)return int(plant->mIsAsleep);
            if(field==16)return int(SandboxFactions::Charmed(plant));
            if(field==11)return plant->mLaunchCounter; // Native production progress, read-only.
            if(field==10)return MemeCharacters::Data(plant,5); // Reserved legacy diagnostic.
            if(field==7)return MemeCharacters::Data(plant,4);
            if(field==8)return int(plant->mState);
            if(field==9)return plant->mStateCountdown;
            if(field==3)return plant->mPlantHealth;
            if(field>=4)return SandboxPlants::HeatData(plant,field-4);
            return field == 0 ? SandboxPlants::Type(plant) : field == 1 ? plant->mPlantCol : plant->mRow;
        }
    }
    return -1;
}
