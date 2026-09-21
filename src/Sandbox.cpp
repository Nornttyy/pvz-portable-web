// Original local sandbox extension. SPDX-License-Identifier: LGPL-3.0-or-later
#include "Sandbox.h"
#include "SandboxRules.h"
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
#include <algorithm>
#include <memory>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

bool gSandboxEnabled = false;
static bool paused = true, stepOnce = false, awake = true;
static int mapType = 0, escaped = 0;
static int sessionRevision = 0;
static std::unique_ptr<PlayerInfo> sandboxProfile;
static PlayerInfo* adventureProfile = nullptr;
static GameMode previousMode = GAMEMODE_ADVENTURE;
static bool previousEasyPlanting = false;
static double previousSpeed = 1;

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
    gSandboxEnabled = true;
    awake = true;
    SandboxStart(0);
    return true;
}

bool SandboxExit() {
    auto* app = gLawnApp;
    if (!app || !gSandboxEnabled) return false;
    // Dispose the sandbox while save/delete guards are still active.
    app->mBoardResult = BOARDRESULT_NONE;
    app->KillBoard();
    SandboxPlants::Reset();
    SandboxZombies::Reset();
    app->mPlayerInfo = adventureProfile;
    adventureProfile = nullptr;
    app->mGameMode = previousMode;
    app->mEasyPlantingCheat = previousEasyPlanting;
    app->mUpdateMultiplier = previousSpeed;
    gSandboxEnabled = false;
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
    app->KillBoard();
    SandboxPlants::Reset();
    SandboxZombies::Reset();
    if (!sandboxProfile) sandboxProfile = std::make_unique<PlayerInfo>();
    auto* profile = sandboxProfile.get();
    profile->mName = "Sandbox";
    profile->mId = 1;
    profile->mFinishedAdventure = 1;
    profile->mLevel = map == 1 ? 28 : 8;
    app->mPlayerInfo = profile;
    app->mGameMode = GAMEMODE_ADVENTURE;
    app->mEasyPlantingCheat = true;
    app->MakeNewBoard();
    auto* board = app->mBoard;
    board->InitLevel();
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
    board->Move(0, 0);
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
    if (!gSandboxEnabled) return;
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
static int PlacePlant(Board* board, int type, int col, int row) {
    if (!SandboxRules::ValidPlant(type) || !SandboxRules::ValidCell(col, row, mapType == 1)) return -2;
    if (PlantCount(board) >= SandboxRules::MaxPlants) return -3;
    const auto seed = static_cast<SeedType>(SandboxPlants::Base(type));
    if (seed == SEED_COBCANNON && col >= 8) return -4;
    if (seed == SEED_CATTAIL && !board->IsPoolSquare(col, row)) return -4;
    if (board->CanPlantAt(col, row, seed) != PLANTING_OK) return -4;
    Plant::PreloadPlantResources(seed);
    PlantsOnLawn existing{};
    board->GetPlantsOnLawn(col, row, &existing);
    if (Plant::IsUpgrade(seed)) {
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
    if (awake && plant->mIsAsleep) plant->SetSleeping(false);
    board->MarkAllDirty();
    return 1;
}
static int Spawn(Board* board, int type, int col, int row) {
    if (!SandboxRules::ValidZombie(type) || !SandboxRules::ValidCell(col, row, mapType == 1)) return -2;
    if (ZombieCount(board) >= SandboxRules::MaxZombies || board->mZombies.mSize >= board->mZombies.mMaxSize - 8) return -3;
    auto zombieType = static_cast<ZombieType>(SandboxZombies::Base(type));
    const bool water = board->IsPoolSquare(col, row);
    if (water && !Zombie::ZombieTypeCanGoInPool(zombieType) && zombieType != ZOMBIE_BALLOON) return -5;
    if (!water && (zombieType == ZOMBIE_SNORKEL || zombieType == ZOMBIE_DOLPHIN_RIDER || zombieType == ZOMBIE_DUCKY_TUBE)) return -5;
    Zombie::PreloadZombieResources(zombieType);
    auto* zombie = board->AddZombieInRow(zombieType, row, Zombie::ZOMBIE_WAVE_DEBUG);
    if (!zombie) return -3;
    zombie->mPosX = static_cast<float>(board->GridToPixelX(col, row) + 10);
    zombie->mX = static_cast<int>(zombie->mPosX);
    SandboxZombies::Assign(zombie,type);
    zombie->UpdateReanim();
    board->MarkAllDirty();
    return 1;
}

extern "C" EMSCRIPTEN_KEEPALIVE int pvz_sandbox_command(int command, int type, int col, int row) {
    auto* board = ActiveBoard();
    if (!board) return -1;
    switch (command) {
    case 0: return 1 | (paused ? 2 : 0) | (mapType == 1 ? 4 : 0) | (awake ? 8 : 0);
    case 1: return PlacePlant(board, type, col, row);
    case 2: return Spawn(board, type, col, row);
    case 3:
        if (!SandboxRules::ValidCell(col, row, mapType == 1)) return -2;
        for (auto* plant : board->mPlants) if (!plant->mDead && plant->mRow == row && (plant->mPlantCol == col || (plant->mSeedType == SEED_COBCANNON && plant->mPlantCol + 1 == col))) plant->Die();
        for (auto* item : board->mGridItems) if (!item->mDead && item->mGridX == col && item->mGridY == row) item->GridItemDie();
        board->ProcessDeleteQueue(); board->MarkAllDirty(); return 1;
    case 4: paused = type != 0; board->mPaused = paused; return 1;
    case 5:
        if (!SandboxRules::ValidSpeed(type)) return -2;
        gLawnApp->mUpdateMultiplier = type; return 1;
    case 6: ClearEnemies(board); return 1;
    case 7:
        SandboxStart(mapType); return 1; // Also resets ice, craters and ongoing instant effects.
    case 8: if (!SandboxRules::ValidMap(type)) return -2; SandboxStart(type); return 1;
    case 9: return PlantCount(board);
    case 10: return ZombieCount(board);
    case 11: {
        if (!SandboxRules::ValidZombie(type)) return -2;
        int added = 0;
        for (int y = 0; y < (mapType == 1 ? 6 : 5); ++y) if (Spawn(board, type, 8, y) > 0) ++added;
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
    default: return -2;
    }
}

extern "C" EMSCRIPTEN_KEEPALIVE int pvz_sandbox_plant_data(int index, int field) {
    auto* board = ActiveBoard();
    if (!board || index < 0 || index >= SandboxRules::MaxPlants || field < 0 || field > 2) return -1;
    for (auto* plant : board->mPlants) {
        if (plant->mDead) continue;
        if (index-- == 0) return field == 0 ? SandboxPlants::Type(plant) : field == 1 ? plant->mPlantCol : plant->mRow;
    }
    return -1;
}
