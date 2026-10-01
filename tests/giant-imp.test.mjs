import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile,writeFile,mkdtemp} from 'node:fs/promises';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
const root=new URL('../',import.meta.url).pathname,run=promisify(execFile);
async function compile(source){const dir=await mkdtemp(join(tmpdir(),'pvz-giant-imp-')),cpp=join(dir,'test.cpp'),bin=join(dir,'test');await writeFile(cpp,source);await run(process.env.CXX||'c++',['-std=c++20','-Isrc',cpp,'-o',bin],{cwd:root});return(await run(bin)).stdout;}
test('giant imp replaces only independent land wave entries from 3-2; giant-thrown imps stay native',async()=>{
 const source=await readFile(join(root,'src/MemeAdventure.cpp'),'utf8');
 const pick=source.slice(source.indexOf('int PickWaveZombie('),source.indexOf('std::string_view Translate('));
 const board=await readFile(join(root,'src/Lawn/Board.cpp'),'utf8');
 const spawn=board.slice(board.indexOf('Zombie* Board::AddZombieInRow('),board.indexOf('Zombie* Board::AddZombie('));
 assert.ok(spawn.indexOf('PickWaveZombie')<spawn.indexOf('ZombieInitialize'));
 assert.match(spawn,/if \(SandboxZombies::Find\(aWaveIdentity\)\) SandboxZombies::Assign\(aZombie, aWaveIdentity\)/);
 assert.match(await compile(`#include "SandboxZombies.h"
#include <vector>
#include <cassert>
#include <iostream>
class Zombie {public:int id=0,mFromWave=0;};
class Board {public:int mLevel=22;bool pool=false;std::vector<Zombie*> mZombies;bool IsPoolSquare(int,int row){return pool&&(row==2||row==3);}};
namespace SandboxZombies {bool IsGiantImp(const Zombie* z){return z->id==GiantImp;}}
namespace MemeAdventure {bool enabled=true;bool RosterEnabled(){return enabled;}${pick}}
int main(){
 for(int level:{1,21,22,23,30,49})for(int base:{0,1,2,23,24,32})for(int wave=-3;wave<30;++wave)for(int row=0;row<6;++row)for(bool pool:{false,true})for(bool enabled:{false,true}){
  Board b;b.mLevel=level;b.pool=pool;MemeAdventure::enabled=enabled;
  const bool eligible=enabled&&SandboxZombies::GiantImpWave(level,base,wave)&&!b.IsPoolSquare(0,row);
  assert(MemeAdventure::PickWaveZombie(&b,base,row,wave)==(eligible?215:base));
  Zombie same{215,wave};b.mZombies={&same};assert(MemeAdventure::PickWaveZombie(&b,base,row,wave)==base);
  same.mFromWave=wave-1;assert(MemeAdventure::PickWaveZombie(&b,base,row,wave)==(eligible?215:base));
 }
 assert(SandboxZombies::Find(215)->base==24&&SandboxZombies::Find(215)->unlock==22);
 assert(MemeAdventure::PickWaveZombie(nullptr,0,0,2)==0);
 std::cout<<"Independent 3-2 giant imp waves passed\\n";
}`),/Independent 3-2/);
});
test('all independent zombie portraits occupy distinct native almanac cells',async()=>{
 const s=await readFile(join(root,'src/Lawn/Widget/AlmanacDialog.cpp'),'utf8');
 const position=s.slice(s.indexOf('void AlmanacDialog::GetZombiePosition('),s.indexOf('ZombieType AlmanacDialog::ZombieHitTest('));
 assert.match(await compile(`#include "ConstEnums.h"
#include "SandboxZombies.h"
#include <set>
#include <utility>
#include <cassert>
#include <iostream>
struct AlmanacDialog{static void GetZombiePosition(ZombieType,int&,int&);};
${position}
int main(){std::set<std::pair<int,int>> cells;
 for(int id=0;id<26;++id){int x,y;AlmanacDialog::GetZombiePosition(ZombieType(id),x,y);assert(cells.emplace(x,y).second);}
 for(auto d:SandboxZombies::Definitions){int x,y;AlmanacDialog::GetZombiePosition(ZombieType(d.id),x,y);assert(cells.emplace(x,y).second);assert(x>=22&&x+63<=440&&y>=86&&y+63<567);}
 std::cout<<"Native almanac cells fit all originals\\n";
}`),/Native almanac cells/);
});
test('jaw impact kills once after windup, pauses with ice, and preserves native plant interactions',async()=>{
 const s=await readFile(join(root,'src/Lawn/Zombie.cpp'),'utf8');
 const update=s.slice(s.indexOf('\tif (SandboxZombies::IsGiantImp(this)',s.indexOf('void Zombie::UpdateZombieImp()')),s.indexOf('\tif (mZombiePhase == ZombiePhase::PHASE_IMP_GETTING_THROWN)',s.indexOf('void Zombie::UpdateZombieImp()')));
 const eat=s.slice(s.indexOf('void Zombie::EatPlant('),s.indexOf('void Zombie::EatZombie('));
 assert.match(s,/mPhaseCounter > 0 && !IsImmobilizied\(\) && !SandboxZombies::HasInteraction\(this\)/);
 assert.match(s,/if \(SandboxZombies::IsGiantImp\(this\) && int\(mZombiePhase\) == SandboxZombies::JawSmash\) return;/);
 assert.match(await compile(`#include "ConstEnums.h"
#include "SandboxZombies.h"
#include <cassert>
#include <iostream>
constexpr int DAMAGE_PER_EAT=4,SOUND_GULP=0;
enum PlantState{STATE_NOTREADY,STATE_FLOWERPOT_INVULNERABLE,STATE_LILYPAD_INVULNERABLE,STATE_SQUASH_LOOK,STATE_SQUASH_PRE_LAUNCH};
enum FoleyType{FOLEY_THUMP};enum ZombieAttackType{ATTACKTYPE_CHEW};
struct Plant {SeedType mSeedType=SEED_WALLNUT;PlantState mState=STATE_NOTREADY;int mPlantHealth=4000,mPlantCol=5,mRow=0,mRecentlyEatenCountdown=0,mX=0,mY=0;bool mDead=false,mIsAsleep=false;int specials=0;void Die(){mDead=true;}void DoSpecial(){++specials;}};
struct App {int sounds=0;bool IsIZombieLevel(){return false;}bool IsFirstTimeAdventureMode(){return false;}void PlaySample(int){}void PlayFoley(FoleyType){++sounds;}};
struct Challenge {int kills=0;void ZombieAtePlant(Plant*){++kills;}};
struct Board {Challenge challenge;Challenge* mChallenge=&challenge;int mPlantsEaten=0,mLevel=22;struct{int mSize=1;}mPlants;bool ladder=false;bool GetLadderAt(int,int){return ladder;}void AddCoin(int,int,CoinType,CoinMotion){}void DisplayAdvice(const char*,MessageStyle,AdviceType){}};
class Zombie {public:int id=215,mPhaseCounter=0,mZombieAge=0,mChilledCounter=0,mUseLadderCol=-1,mJustGotShotCounter=0;bool mHasHead=true,mMindControlled=false,mYuckyFace=false,mIsEating=false;ZombiePhase mZombiePhase=PHASE_ZOMBIE_NORMAL;ZombieHeight mZombieHeight=HEIGHT_ZOMBIE_NORMAL;ZombieType mZombieType=ZOMBIE_IMP;Board* mBoard;App* mApp;Plant* target=nullptr;
 void StopEating(){mIsEating=false;}void StartEating(){mIsEating=true;}Plant* FindPlantTarget(ZombieAttackType){return target&&!target->mDead?target:nullptr;}
 void EatPlant(Plant*);void UpdateZombieImp();void Tick(bool ice=false){++mZombieAge;if(ice)return;if(mPhaseCounter>0)--mPhaseCounter;UpdateZombieImp();}};
namespace SandboxZombies {bool IsGiantImp(const Zombie* z){return z->id==215;}bool StealPlant(Zombie*,Plant*){return false;}}
${eat}
void Zombie::UpdateZombieImp(){${update}}
int main(){
 App app;Board b;Plant p;Zombie z;z.mBoard=&b;z.mApp=&app;z.target=&p;
 z.EatPlant(&p);assert(p.mPlantHealth==4000&&z.mPhaseCounter==90&&z.mIsEating);
 for(int i=0;i<54;++i)z.Tick();assert(p.mPlantHealth==4000&&z.mPhaseCounter==36);
 for(int i=0;i<200;++i)z.Tick(true);assert(p.mPlantHealth==4000&&z.mPhaseCounter==36);
 z.Tick();assert(p.mDead&&p.mPlantHealth==0&&b.mPlantsEaten==1&&b.challenge.kills==1&&app.sounds==1);
 for(int i=0;i<35;++i)z.Tick();assert(z.mZombiePhase==PHASE_ZOMBIE_NORMAL&&!z.mIsEating&&b.mPlantsEaten==1);
 // Lost target isn't attacked through stale pointers; lost head cancels.
 p=Plant{};z.EatPlant(&p);z.target=nullptr;for(int i=0;i<90;++i)z.Tick();assert(!p.mDead);
 z.target=&p;z.EatPlant(&p);z.mHasHead=false;z.Tick();assert(z.mZombiePhase==PHASE_ZOMBIE_NORMAL&&!p.mDead&&!z.mIsEating);z.mHasHead=true;
 // Native invulnerable explosives/pots and ladders remain native.
 for(SeedType seed:{SEED_CHERRYBOMB,SEED_JALAPENO,SEED_ICESHROOM,SEED_HYPNOSHROOM}){p=Plant{};p.mSeedType=seed;z.EatPlant(&p);assert(!p.mDead&&p.mPlantHealth==4000&&z.mZombiePhase==PHASE_ZOMBIE_NORMAL);}
 p=Plant{};b.ladder=true;z.EatPlant(&p);assert(z.mZombieHeight==HEIGHT_UP_LADDER&&!p.mDead);b.ladder=false;z.mZombieHeight=HEIGHT_ZOMBIE_NORMAL;
 // Ordinary/thrown imp still takes normal bites, never jaw-smashes.
 p=Plant{};z.id=24;z.EatPlant(&p);assert(p.mPlantHealth==3996&&z.mZombiePhase==PHASE_ZOMBIE_NORMAL);
 // Chilled impact is not skipped merely because zombie age is odd.
 p=Plant{};z.id=215;z.mChilledCounter=100;z.EatPlant(&p);z.mZombieAge=0;for(int i=0;i<55;++i)z.Tick();assert(p.mDead);
 std::cout<<"Jaw windup, impact, freeze, native interactions passed\\n";
}`),/Jaw windup/);
});
